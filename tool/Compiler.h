#pragma once

#include <string>
#include <vector>

#include "util/CmdParser.h"

namespace Argonauts {
namespace Tool {
struct File;

class AbstractCompiler;

class Compiler
{
public:
	explicit Compiler();

	std::string input;

	void setup(Util::CLI::ParserBuilder::Ptr &builder);
	bool run(const Util::CLI::Parser &parser, AbstractCompiler *compiler);
	Util::CLI::Execution listCompilers(const Util::CLI::Parser &);

private:
	std::vector<AbstractCompiler *> m_compilers;
};
}
}
