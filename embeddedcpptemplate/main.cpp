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

#include "util/CmdParser.h"
#include "util/Error.h"
#include "util/TermUtil.h"
#include "util/FSUtil.h"
#include "util/StringUtil.h"

#include <iostream>
#include <boost/filesystem.hpp>

using namespace Argonauts::Util;

/* This function will process the given `data` and produce two result strings: a header and a source file.
 *
 * The given input data should start of with metadata rows in the format `<key>: <value>`. `<key>` can be `arguments`
 * or `includes`. In the case of `arguments`, the `<value>` is copied verbatim as the argument list for the generated
 * function. `includes` are split up at ',' and then each is added as an #include line in the final header. Rows
 * that do not start with a recognized key are simply skipped, this can be used to add a license header or similar.
 *
 * After those rows an empty row must follow, after which the actual template contents are. These can be anything, but
 * the following are handled specially:
 * * <%= expression %>
 *     * expression has to result in a std::string, char*, const char*, or any type that std::to_string can take. The
 *       output will be appended to the runtime output at runtime.
 * * <% statement %>
 *     * Anything put here will be copied directly to the output when the template is processed.
 * * Everything else is copied unchanged.
 *
 * Example:
 * ```
 * arguments: const std::string &title, std::vector<std::string> &guides
 * includes: <string>, <vector>
 *
 * Welcome to <%= title %>!
 *
 * Some of our guides are:
 * <% for (const std::string &guide : guides) { %>
 * <%= guide %>
 * <% } %>
 * ```
 * will, when run with the arguments "the Zoo", {"Arthur", "Zaphod", "Trillian"} generate:
 * ```
 * Welcome to the Zoo!
 *
 * Some of our guides are:
 * Arthur
 * Zaphod
 * Trillian
 * ```
 *
 * It is important to note that quite a bit of whitespace (including newlines) will be left in the final output.
 */
std::pair<std::string, std::string> processEct(const std::string &filename, const std::string &data)
{
	std::string argumentsRow, includesRow;
	std::size_t index = 0;
	while (true) {
		const std::size_t rowEnd = data.find('\n', index);
		if (rowEnd == index || rowEnd == std::string::npos) {
			// header done on empty row
			break;
		} else {
			if (data.find("arguments: ", index) == index) {
				argumentsRow = data.substr(index, rowEnd - index).substr(std::string("arguments: ").size());
			} else if (data.find("includes: ", index) == index) {
				includesRow = data.substr(index, rowEnd - index).substr(std::string("includes: ").size());
			}
		}
		index = rowEnd + 1;
	}
	std::string header, source;
	header += std::string("#pragma once\n")
			+ "#include <string>\n";
	for (const std::string &include : String::splitStrings(includesRow, ", ")) {
		header += "#include " + include + "\n";
	}
	header += "std::string generate" + boost::filesystem::path(filename).stem().string() + "(" + argumentsRow + ");\n";
	source += "#include \"" + boost::filesystem::path(filename).filename().string() + ".h\"\n"
			+ "\n"
			+ "static inline std::string toString(const std::string &string) { return string; }\n"
			+ "static inline std::string toString(const char *string) { return string; }\n"
			+ "static inline std::string toString(char *string) { return string; }\n"
			+ "template <typename T> static inline std::string toString(const T val) { return std::to_string(val); }\n"
			+ "\n"
			+ "std::string generate" + boost::filesystem::path(filename).stem().string() + "(" + argumentsRow + ")\n"
			+ "{\n"
			+ "\tstd::string out;\n";
	while (true) {
		const std::size_t next = data.find('<', index);
		source += "\tout += toString(\"" + String::replaceAll(String::replaceAll(String::replaceAll(data.substr(index, next - index), "\n", "\\n"), "\t", "\\t"), "\"", "\\\"") + "\");\n";
		if (next == std::string::npos) {
			break;
		}
		if (data.size() > next && data.at(next + 1) == '%') {
			const std::size_t end = data.find("%>", next);
			if (end == std::string::npos) {
				throw Error("'<%' or '<%=' token without ending", next, Error::Source(data, filename));
			}
			if (data.at(next + 2) == '=') {
				source += "\tout += toString(" + data.substr(next + 3, end - (next + 3)) + ");\n";
			} else {
				source += '\t' + data.substr(next + 2, end - (next + 2)) + "\n";
			}
			index = end + 2;
		} else {
			source += "\tout += '<';\n";
			index = next + 1;
		}
	}
	source += std::string("\treturn out;\n")
			+ "}\n";
	return {header, source};
}

int main(int argc, const char **argv)
{
	CLI::ParserBuilder::Ptr builder = std::make_shared<CLI::ParserBuilder>("embeddedcpptemplate", "1.0", "This tool transforms embeddedcpptemplates (.ect files) into actual C++ code");
	builder->addOption({"output", "o"}, "The directory to where to output the resulting files")->withRequiredArg("DIR")->beingRequired();
	builder->withPositionalArgument("INPUT", "List of input files", CLI::Subcommand::Repeatable);
	builder->then([](const CLI::Parser &parser)
	{
		const boost::filesystem::path outdir = boost::filesystem::path(parser.option<std::string>("output"));
		if (!boost::filesystem::exists(outdir)) {
			if (!boost::filesystem::create_directories(outdir)) {
				throw Exception("Unable to create output directory");
			}
		} else if (!boost::filesystem::is_directory(outdir)) {
			throw Exception("Output directory already exists but is not a directory");
		}
		for (const std::string &infile : parser.positionalArguments("INPUT")) {
			if (!boost::filesystem::exists(infile)) {
				throw Exception(std::string("No such input file: ") + infile);
			} else if (!boost::filesystem::is_regular_file(infile)) {
				throw Exception(std::string("Given file is not a regular file: ") + infile);
			}
			const boost::filesystem::path filename = boost::filesystem::path(infile).filename();
			const std::string contents = FS::readFile(infile);
			const std::pair<std::string, std::string> outdata = processEct(infile, contents);
			FS::writeFile((outdir / filename).string() + ".h", outdata.first);
			FS::writeFile((outdir / filename).string() + ".cpp", outdata.second);
		}
		return CLI::Execution::ExitSuccess;
	});
	builder->addVersionOption();
	builder->addHelpOption();
	builder->addListCommand();
	builder->addHelpCommand();
	try {
		return builder->build().parse(argc, argv);
	} catch (Error &e) {
		std::cerr << Term::fg(Term::Red, e.errorMessage()) << std::endl;
		return -1;
	} catch (Exception &e) {
		std::cerr << Term::fg(Term::Red, e.what()) << std::endl;
		return -1;
	}
}
