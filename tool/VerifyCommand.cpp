#include "VerifyCommand.h"

#include <iostream>

#include "util/FSUtil.h"
#include "util/TermUtil.h"
#include "util/Error.h"
#include "Lexer.h"
#include "Parser.h"

namespace Argonauts {
namespace Tool {
VerifyCommand::VerifyCommand() {}

void VerifyCommand::setup(Util::CLI::ParserBuilder::Ptr &builder)
{
	builder->addSubcommand({"verify", "v"}, "Verifies that the syntax of a given list of grammar files is correct")->setup([this](Util::CLI::Subcommand::Ptr &cmd)
	{
		cmd->withPositionalArgument("INPUTS", "A list of ProjectArgonauts grammar files to check", Util::CLI::Subcommand::Repeatable);
		cmd->then([this](const Util::CLI::Parser &parser) { return run(parser); });
	});
}
Util::CLI::Execution VerifyCommand::run(const Util::CLI::Parser &parser)
{
	bool haveError = false;
	for (const std::string &file : parser.positionalArguments("INPUTS")) {
		try {
			Parser(Lexer().consume(FSUtil::readFile(file), file)).process();
			std::cout << TermUtil::fg(TermUtil::Green, TermUtil::style(TermUtil::Bold, file) + " is valid") << std::endl;
		} catch (Error &error) {
			std::cerr << TermUtil::fg(TermUtil::Red, error.errorMessage()) << std::endl;
			haveError = true;
		}
	}
	return haveError ? Util::CLI::Execution::ExitFailure : Util::CLI::Execution::ExitSuccess;
}

}
}
