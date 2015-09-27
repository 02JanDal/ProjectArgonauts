/*
 * Copyright 2015 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CppCompiler.h"

#include <unordered_map>
#include <unordered_set>

#include <ostream>
#include <boost/filesystem.hpp>

#include "util/CmdParser.h"
#include "tool/DataTypes.h"
#include "cpp/TypeProviders.h"

#include "templates/EnumHeader.ect.h"
#include "templates/EnumSource.ect.h"
#include "templates/StructHeader.ect.h"
#include "templates/StructSource.ect.h"
#include "templates/UsingHeader.ect.h"
#include "templates/UsingSource.ect.h"

#include "tool_config.h"

namespace Argonauts {
namespace Tool {
void CppCompiler::setup(std::shared_ptr<Util::CLI::Subcommand> &builder)
{
	builder->addOption({"types", "t"}, "Which kind of data types to use.")
			->withRequiredArg("TYPES")
			->requireFromSet({"qt", "stl"})
			->applyTo(this, &CppCompiler::m_dataTypes);
	builder->addOption({"output", "out", "o"}, "Where to put the resulting files (give a directory)")
			->withRequiredArg("DIR")
			->applyTo(this, &CppCompiler::m_directory)
			->beingRequired();
}

static std::vector<std::string> listTypesRecursive(const Type::Ptr &ptr)
{
	std::vector<std::string> types;
	if (ptr->name == "List") {
		types.push_back(ptr->templateArguments.front()->name);
	}
	for (const Type::Ptr &arg : ptr->templateArguments) {
		const auto argTypes = listTypesRecursive(arg);
		std::copy(argTypes.begin(), argTypes.end(), std::back_inserter(types));
	}
	return types;
}

static void writeEnumHeader(std::ostream &out, const Enum &enumeration, TypeProvider *types)
{
	out << generateEnumHeader(ARG_TOOL_VERSION, enumeration, types);
}
static void writeEnumSource(std::ostream &out, const Enum &enumeration, TypeProvider *types, const std::string &headerFilename)
{
	out << generateEnumSource(ARG_TOOL_VERSION, enumeration, types, headerFilename);
}
static void writeStructHeader(std::ostream &out, const Struct &structure, TypeProvider *types)
{
	// collect all required headers. use a set to prevent duplicates
	std::unordered_set<std::string> typeHeaders;
	std::vector<std::string> constructorArgs, constructorInitList, builderCopyInitList, builderBuildArgList;
	for (const Attribute &attribute : structure.members) {
		for (const std::string &type : attribute.type->namesRecursive()) {
			typeHeaders.insert(types->headerForType(type));
		}

		constructorArgs.push_back(std::string("const ") + types->fullType(attribute) + " &" + attribute.name);
		constructorInitList.push_back(std::string("m_") + attribute.name + "(" + attribute.name + ")");
		builderCopyInitList.push_back(std::string("m_") + attribute.name + "(original." + attribute.name + "())");
		builderBuildArgList.push_back(std::string("m_") + attribute.name);
	}

	typeHeaders.erase(std::string());
	out << generateStructHeader(ARG_TOOL_VERSION, structure, types, typeHeaders, builderCopyInitList, builderBuildArgList, constructorArgs, constructorInitList);
}
static void writeStructSource(std::ostream &out, const Struct &structure, TypeProvider *types, const std::string &headerFilename)
{
	out << generateStructSource(ARG_TOOL_VERSION, structure, types, headerFilename);
}
static void writeUsingHeader(std::ostream &out, const Using &alias, TypeProvider *types)
{
	out << generateUsingHeader(ARG_TOOL_VERSION, alias, types);
}
static void writeUsingSource(std::ostream &out, const Using &alias, TypeProvider *types, const std::string &headerFilename)
{
	out << generateUsingSource(ARG_TOOL_VERSION, alias, types, headerFilename);
}

bool CppCompiler::run(const Util::CLI::Parser &, const File &file)
{
	TypeProvider *provider;
	if (m_dataTypes == "qt") {
		provider = new QtTypeProvider;
	} else if (m_dataTypes == "stl") {
		provider = new STLTypeProvider;
	} else {
		std::terminate();
	}

	for (const Enum &enumeration : file.enums) {
		openFileAndCall(filenameFor(enumeration.name, true), &writeEnumHeader, enumeration, provider);
		openFileAndCall(filenameFor(enumeration.name, false), &writeEnumSource, enumeration, provider, boost::filesystem::path(filenameFor(enumeration.name, true)).filename().string());
	}
	for (const Struct &structure : file.structs) {
		openFileAndCall(filenameFor(structure.name, true), &writeStructHeader, structure, provider);
		openFileAndCall(filenameFor(structure.name, false), &writeStructSource, structure, provider, boost::filesystem::path(filenameFor(structure.name, true)).filename().string());
	}
	for (const Using &alias : file.usings) {
		openFileAndCall(filenameFor(alias.name, true), &writeUsingHeader, alias, provider);
		openFileAndCall(filenameFor(alias.name, false), &writeUsingSource, alias, provider, boost::filesystem::path(filenameFor(alias.name, true)).filename().string());
	}
	return true;
}

std::string CppCompiler::filenameFor(const std::string &type, const bool header) const
{
	return m_directory + '/' + type + ".arg." + (header ? "h" : "cpp");
}
}
}
