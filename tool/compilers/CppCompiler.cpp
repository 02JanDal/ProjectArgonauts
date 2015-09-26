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

#include <fstream>
#include <ostream>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem.hpp>

#include "util/CmdParser.h"
#include "tool/DataTypes.h"
#include "cpp/TypeProviders.h"

#include "templates/EnumHeader.ect.h"
#include "templates/EnumSource.ect.h"
#include "templates/StructHeader.ect.h"
#include "templates/StructSource.ect.h"

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

static void writeEnumHeader(std::ofstream &out, const Enum &enumeration, TypeProvider *types)
{
	out << generateEnumHeader(ARG_TOOL_VERSION, enumeration, types);
}
static void writeEnumSource(std::ofstream &out, const Enum &enumeration, TypeProvider *types, const std::string &headerFilename)
{
	out << generateEnumSource(ARG_TOOL_VERSION, enumeration, types, headerFilename);
}
static void writeStructHeader(std::ofstream &out, const Struct &structure, TypeProvider *types)
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
static void writeStructSource(std::ofstream &out, const Struct &structure, TypeProvider *types, const std::string &headerFilename)
{
	out << generateStructSource(ARG_TOOL_VERSION, structure, types, headerFilename);
}

template <typename Type, typename Func, typename... Args>
void openFileAndCall(const std::string &filename, const Type &data, Func &&func, Args... args)
{
	using namespace boost;

	filesystem::path path = filesystem::path(filename).parent_path();
	if (!filesystem::exists(path)) {
		if (!filesystem::create_directories(path)) {
			throw Util::Exception(std::string("Unable to create parent directories for ") + filename);
		}
	}
	if (filesystem::exists(filesystem::path(filename)) && filesystem::is_regular_file(path)) {
		throw Util::Exception(std::string("'") + filename + "' already exists but is not a file");
	}

	std::ofstream stream(filename);
	if (!stream.good()) {
		throw Util::Exception(std::string("Unable to open file '") + filename + "' for writing");
	}
	func(stream, data, args...);
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
		openFileAndCall(filenameFor(enumeration.name, true), enumeration, &writeEnumHeader, provider);
		openFileAndCall(filenameFor(enumeration.name, false), enumeration, &writeEnumSource, provider, boost::filesystem::path(filenameFor(enumeration.name, true)).filename().string());
	}
	for (const Struct &structure : file.structs) {
		openFileAndCall(filenameFor(structure.name, true), structure, &writeStructHeader, provider);
		openFileAndCall(filenameFor(structure.name, false), structure, &writeStructSource, provider, boost::filesystem::path(filenameFor(structure.name, true)).filename().string());
	}
	return true;
}

std::string CppCompiler::filenameFor(const std::string &type, const bool header) const
{
	return m_directory + '/' + type + ".arg." + (header ? "h" : "cpp");
}
}
}
