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

#include "VerifyCommand.h"

#include <iostream>

#include "util/FSUtil.h"
#include "util/TermUtil.h"
#include "util/Error.h"
#include "DataTypes.h"

namespace Argonauts {
namespace Tool {
VerifyCommand::VerifyCommand() {}

void VerifyCommand::setup(Util::CLI::ParserBuilder::Ptr &builder)
{
	builder->addSubcommand({"verify", "v"}, "Verifies that the syntax of a given list of grammar files is correct")->setup([this](Util::CLI::Subcommand::Ptr &cmd)
	{
		cmd->addOption({"dump", "d"}, "If given, dumps the result");
		cmd->withPositionalArgument("INPUTS", "A list of ProjectArgonauts grammar files to check", Util::CLI::Subcommand::Repeatable);
		cmd->then([this](const Util::CLI::Parser &parser) { return run(parser); });
	});
}
Util::CLI::Execution VerifyCommand::run(const Util::CLI::Parser &parser)
{
	using namespace Util::Term;

	bool haveError = false;
	for (const std::string &file : parser.positionalArguments("INPUTS")) {
		try {
			auto f = lexAndParse(Util::FS::readFile(file), file, ResolveIncludes | ResolveAliases);
			std::cout << fg(Green, style(Bold, file) + " is valid") << std::endl;
			if (parser.hasOption("dump")) {
				std::cout << "It contains " << style(Bold, std::to_string(f.structs.size()) + " structs") << " and " << style(Bold, std::to_string(f.enums.size()) + " enums") << std::endl;
			}
		} catch (Util::Error &error) {
			std::cerr << fg(Red, error.errorMessage()) << std::endl;
			haveError = true;
		}
	}
	return haveError ? Util::CLI::Execution::ExitFailure : Util::CLI::Execution::ExitSuccess;
}

}
}
