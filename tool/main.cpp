#include "Lexer.h"
#include "Parser.h"

#include <vector>
#include <functional>
#include <iostream>

#include "util/Util.h"
#include "util/StringUtil.h"
#include "util/CmdParser.h"
#include "util/TermUtil.h"

#include "DataTypes.h"
#include "Compiler.h"
#include "DocGenerator.h"
#include "Importer.h"
#include "tool_config.h"

int main(int argc, const char **argv)
{
	Compiler compiler;
	Importer importer;

	CLI::ParserBuilder builder("argonauts", ARG_TOOL_VERSION, "This tool allows to to perform various actions related to Argonaut files");
	builder.addSubcommand({"compile", "c"}, "Compiles Argonaut files to language-specific code").setup([&compiler](CLI::Subcommand &cmd)
	{
		cmd.withPositionalArgument("file", "The input file to compile").applyTo(compiler, &Compiler::input);
		cmd.addOption({"output", "o"}, "The name of the output file").beingRequired().applyTo(compiler, &Compiler::output);
		cmd.then([&compiler]() { return compiler.run() ? CLI::Execution::ExitSuccess : CLI::Execution::ExitFailure; });
	});
	builder.addSubcommand({"import", "i"}, "Creates an Argnaut file from JSON-Schema").setup([&importer](CLI::Subcommand &cmd)
	{
		cmd.withPositionalArgument("schema", "The JSON-Schema(s) file to import", CLI::Subcommand::Repeatable).applyTo(&importer, &Importer::schemas);
		cmd.addOption({"output", "o"}, "If name of an existing directory or ending with / each schema will be put in a separate file, otherwise all schemas will be put in the file with this name")
				.beingRequired().applyTo(importer, &Importer::output);
		cmd.addOption({"force", "no-force", "f"}, "If a file with the same name already exists force overwrite it").applyTo(importer, &Importer::force);
		cmd.then([&importer]() { return importer.run() ? CLI::Execution::ExitSuccess : CLI::Execution::ExitFailure; });
	});
	builder.addVersionOption();
	builder.addHelpOption();
	builder.addListCommand();
	builder.addHelpCommand();
	try
	{
		return builder.build().parse(argc, argv);
	}
	catch (ArgonautsException &e)
	{
		std::cerr << TermUtil::fg(TermUtil::Red, e.what()) << std::endl;
		return -1;
	}

//	const std::string filename("../grammar.arg");
//	QFile file(QString::fromStdString(filename));
//	if (!file.open(QFile::ReadOnly))
//	{
//		return -1;
//	}
//	const std::string data = QString::fromUtf8(file.readAll()).toStdString();
//	try
//	{
//		std::vector<Lexer::Token> tokens = Lexer().consume(data);
//		Argonauts::File file = Parser(tokens).process();
//	}
//	catch (Lexer::LexerException &e)
//	{
//		std::cerr << e.what();
//	}
//	catch (Parser::ParserException &e)
//	{
//		std::cerr << e.what() << std::endl;
//		const auto position = Util::lineColumnFromDataAndOffset(data, e.offset);
//		std::cerr << "In " << filename << ":" << position.first << ":" << position.second << std::endl;
//		const std::string line = Util::lineInData(data, e.offset);
//		std::cerr << StringUtil::replaceAll(line, "\t", "    ") << std::endl
//				  << std::string(position.second - 1 + std::count(line.begin(), line.end(), '\t') * 3, ' ') << '^' << std::endl;
//	}

	return 0;
}
