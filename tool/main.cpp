#include "Lexer.h"
#include "Parser.h"

#include <vector>
#include <functional>
#include <iostream>

#include "util/Util.h"
#include "util/StringUtil.h"
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
		std::cerr << TermUtil::fg(TermUtil::Red, e.errorMessage()) << std::endl;
		return -1;
	} catch (ArgonautsException &e) {
		std::cerr << TermUtil::fg(TermUtil::Red, e.what()) << std::endl;
		return -1;
	}
}
