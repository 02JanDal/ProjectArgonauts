#include "RubyCompiler.h"

#include "util/CmdParser.h"
#include "tool/DataTypes.h"

namespace Argonauts {
namespace Tool {
void RubyCompiler::setup(std::shared_ptr<Util::CLI::Subcommand> &builder)
{
	builder->addOption({"ext"}, "If specified, makes use of Ruby native extensions")->applyTo(this, &RubyCompiler::m_nativeExtensions);
}

bool RubyCompiler::run(const Util::CLI::Parser &parser, const File &file)
{
	return false;
}
}
}
