#include "JsonSchemaCompiler.h"

#include "util/CmdParser.h"
#include "tool/DataTypes.h"

namespace Argonauts {
namespace Tool {
void JsonSchemaCompiler::setup(std::shared_ptr<Util::CLI::Subcommand> &builder)
{
}

bool JsonSchemaCompiler::run(const Util::CLI::Parser &parser, const File &file)
{
	return false;
}
}
}
