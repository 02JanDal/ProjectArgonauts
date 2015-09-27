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

#include "CppJSONCompiler.h"

#include <vector>
#include <iostream>

#include "util/CmdParser.h"
#include "util/StringUtil.h"
#include "tool/DataTypes.h"

#include "templates/JSONParserHeader.ect.h"
#include "templates/JSONParserSource.ect.h"
#include "templates/JSONSerializerHeader.ect.h"
#include "templates/JSONSerializerSource.ect.h"

#include "tool_config.h"

namespace Argonauts {
namespace Tool {
void CppJSONCompiler::setup(std::shared_ptr<Util::CLI::Subcommand> &builder)
{
	builder->addOption({"types", "t"}, "Which kind of data types to use.")
			->withRequiredArg("TYPES")
			->requireFromSet({"qt", "stl"})
			->applyTo(this, &CppJSONCompiler::m_dataTypes);
	builder->addOption({"output", "out", "o"}, "Where to put the resulting files (give a directory)")
			->withRequiredArg("DIR")
			->applyTo(this, &CppJSONCompiler::m_directory)
			->beingRequired();
	builder->addOption({"parser", "no-parser"}, "Should a parser be generated? (default yes)");
	builder->addOption({"serializer", "no-serializer"}, "Should a serializer be generated? (default yes)");
	builder->addOption({"root-type"}, "The \"root\" type that should be generated for")->withRequiredArg("TYPE")->beingRequired();
	builder->addOption({"enum-serialization"}, "Determine how enums should be serialized. Default is 'string'.")->withRequiredArg("TYPE", "string")->requireFromSet({"integer", "string"});
	builder->addOption({"enum-parsing"}, "Determine how enums should be parsed. Default is 'both'")->withRequiredArg("TYPE", "both")->requireFromSet({"integer", "string", "both"});
}

static void writeParserHeader(std::ostream &str, const std::string &root, const File &file, TypeProvider *types)
{
	str << generateJSONParserHeader(ARG_TOOL_VERSION, root, file, types);
}
static void writeParserSource(std::ostream &str, const std::string &root, const File &file, TypeProvider *types)
{
	str << generateJSONParserSource(ARG_TOOL_VERSION, root, file, types);
}
static void writeSerializerHeader(std::ostream &str, const std::string &root, const File &file, TypeProvider *types)
{
	str << generateJSONSerializerHeader(ARG_TOOL_VERSION, root, file, types);
}
static void writeSerializerSource(std::ostream &str, const std::string &root, const File &file, TypeProvider *types, const std::string &enumSerialization)
{
	str << generateJSONSerializerSource(ARG_TOOL_VERSION, root, file, types, enumSerialization);
}

bool CppJSONCompiler::run(const Util::CLI::Parser &parser, const File &file)
{
	const std::string rootType = parser.option<std::string>("root-type");
	if (std::find_if(file.structs.begin(), file.structs.end(), [rootType](const Struct &s) { return rootType == s.name.value; }) == file.structs.end()) {
		throw std::runtime_error("Unknown root type (must be a struct)");
	}

	TypeProvider *provider;
	if (m_dataTypes == "qt") {
		provider = new QtTypeProvider;
	} else if (m_dataTypes == "stl") {
		provider = new STLTypeProvider;
	} else {
		std::terminate();
	}

	if (!parser.hasOption("parser") || parser.option<std::string>("parser") == "true") {
		std::cout << "Generating parser for " << rootType << "...\n";
		openFileAndCall(m_directory + '/' + rootType + ".parser.json.arg.h", &writeParserHeader, rootType, file, provider);
		openFileAndCall(m_directory + '/' + rootType + ".parser.json.arg.cpp", &writeParserSource, rootType, file, provider);
	}
	if (!parser.hasOption("serializer") || parser.option<std::string>("serializer") == "true") {
		std::cout << "Generating serializer for " << rootType << "...\n";
		openFileAndCall(m_directory + '/' + rootType + ".serializer.json.arg.h", &writeSerializerHeader, rootType, file, provider);
		openFileAndCall(m_directory + '/' + rootType + ".serializer.json.arg.cpp", &writeSerializerSource, rootType, file, provider, parser.option<std::string>("enum-serialization"));
	}
	return true;
}
}
}
