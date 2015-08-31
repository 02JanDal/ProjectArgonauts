#include "Compiler.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "util/OsUtil.h"
#include "util/TermUtil.h"
#include "util/FSUtil.h"
#include "Lexer.h"
#include "Parser.h"
#include "DataTypes.h"

#include "compilers/CppCompiler.h"
#include "compilers/RubyCompiler.h"
#include "compilers/JsonSchemaCompiler.h"
#include "compilers/DocCompiler.h"

namespace Argonauts {
namespace Tool {
Compiler::Compiler()
{
	m_compilers.push_back(new DocCompiler);
	m_compilers.push_back(new CppCompiler);
	m_compilers.push_back(new RubyCompiler);
	m_compilers.push_back(new JsonSchemaCompiler);
}

void Compiler::setup(Util::CLI::ParserBuilder::Ptr &builder)
{
	builder->addSubcommand({"compile", "c"}, "Compiles Argonaut files to language-specific code")->setup([this](Util::CLI::Subcommand::Ptr &cmd)
	{
		cmd->withPositionalArgument("file", "The input file to compile")->applyTo(this, &Compiler::input);
		cmd->addOption({"list-languages"}, "List available language compilers")->then(this, &Compiler::listCompilers)->makeEarlyExit();
		for (AbstractCompiler *compiler : m_compilers)
		{
			cmd->addSubcommand({compiler->name()}, compiler->help())->setup([this, compiler](Util::CLI::Subcommand::Ptr &cmd)
			{
				compiler->setup(cmd);
				cmd->then([this, compiler](const Util::CLI::Parser &parser) { return run(parser, compiler) ? Util::CLI::Execution::ExitSuccess : Util::CLI::Execution::ExitFailure; });
			});
		}
	});
}

bool Compiler::run(const Util::CLI::Parser &parser, AbstractCompiler *compiler)
{
	const File file = lexAndParse(Util::FS::readFile(input), input, compiler->resolverFlags());
	return compiler->run(parser, file);
}

Util::CLI::Execution Compiler::listCompilers(const Util::CLI::Parser &)
{
	for (AbstractCompiler *compiler : m_compilers)
	{
		std::cout << "\t* " << Util::Term::style(Util::Term::Bold, compiler->name()) << "\t(" << compiler->help() << ")\n";
	}
	return Util::CLI::Execution::ExitSuccess;
}
}
}
