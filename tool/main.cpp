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

#include "Lexer.h"
#include "Parser.h"

#include <vector>
#include <functional>
#include <iostream>

#include "util/Util.h"
#include "util/CmdParser.h"
#include "util/TermUtil.h"
#include "util/Error.h"

#include "DataTypes.h"
#include "Compiler.h"
#include "Importer.h"
#include "VerifyCommand.h"
#include "tool_config.h"

using namespace Argonauts::Util;
using namespace Argonauts::Tool;

int main(int argc, const char **argv)
{
	Compiler compiler;
	Importer importer;
	VerifyCommand verifier;

	CLI::ParserBuilder::Ptr builder = std::make_shared<CLI::ParserBuilder>("argonauts", ARG_TOOL_VERSION, "This tool allows to to perform various actions related to Argonaut files");
	compiler.setup(builder);
	importer.setup(builder);
	verifier.setup(builder);
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
