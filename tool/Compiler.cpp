#include "Compiler.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
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

using namespace boost;

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
	const File file = Parser(Lexer().consume(FSUtil::readFile(input), input)).process();
	return compiler->run(parser, file);
}

Util::CLI::Execution Compiler::listCompilers(const Util::CLI::Parser &)
{
	for (AbstractCompiler *compiler : m_compilers)
	{
		std::cout << "\t* " << TermUtil::style(TermUtil::Bold, compiler->name()) << "\t(" << compiler->help() << ")\n";
	}
	return Util::CLI::Execution::ExitSuccess;
	/*std::vector<std::string> list = candidates();
	if (list.empty()) {
		std::cerr << "No candidates found\n";
		return CLI::Execution::ExitFailure;
	} else {
		std::cout << "Available languages:\n";
		static const std::string prefix = "argonauts-compiler-";
		for (const std::string &candidate : list) {
			const std::size_t pos = candidate.find(prefix);
			std::cout << "\t* " << TermUtil::style(TermUtil::Bold, candidate.substr(pos + prefix.size())) << " \t(" << TermUtil::style(TermUtil::Concealed, candidate) << ")\n";
		}
		return CLI::Execution::ExitSuccess;
	}*/
}

/*static bool examineCandidate(const filesystem::path &path)
{
	const int permissions = filesystem::status(path).permissions();
	return filesystem::exists(path) && filesystem::is_regular_file(path)
	#ifdef OS_UNIX
			&& permissions & (filesystem::owner_exe | filesystem::group_exe | filesystem::others_exe)
	#endif
			;
}
void Compiler::resolveCompiler()
{
	const filesystem::path path(language);
	if (path.is_absolute()) {
		if (!examineCandidate(path)) {
			throw std::runtime_error(boost::str(boost::format("%1% is no such executable file") % language));
		}
	} else {
		const std::vector<std::string> cand = candidates();
		if (cand.empty()) {
			throw std::runtime_error(boost::str(boost::format("No compiler for %1% found") % language));
		} else {
			language = cand.front();
		}
	}
}
std::vector<std::string> Compiler::candidates() const
{
	std::vector<std::string> result;

	auto examineDirectory = [this, &result](const filesystem::path &directory)
	{
		if (language.empty()) {
			for (const filesystem::path &path : filesystem::directory_iterator(directory)) {
				if (path.string().find("argonauts-compiler-") != std::string::npos) {
					if (examineCandidate(path)) {
						result.push_back(path.string());
					}
				}
			}
		} else {
			filesystem::path candidate1 = directory / ("argonauts-compiler-" + language);
			filesystem::path candidate2 = directory / language;
			if (examineCandidate(candidate1)) {
				result.push_back(candidate1.string());
			} else if (examineCandidate(candidate2)) {
				result.push_back(candidate2.string());
			}
		}
	};

#ifdef OS_UNIX
	// search in the same directory as the tool itself
	filesystem::path mypath("/proc/self/exe");
	while (filesystem::is_symlink(mypath)) {
		mypath = filesystem::read_symlink(mypath);
		examineDirectory(mypath.parent_path());
	}
	// TODO[Windows]: Implement this for windows
#endif

	// search the path
	char *PATH = getenv("PATH");
	char *dir;
	for (dir = std::strtok(PATH, ":"); dir; dir = std::strtok(nullptr, ":")) {
		if (filesystem::exists(dir)) {
			examineDirectory(filesystem::path(dir));
		}
	}

	return result;
}*/
}
}
