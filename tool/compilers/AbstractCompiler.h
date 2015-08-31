#pragma once

#include <string>
#include <memory>

namespace Argonauts {
namespace Util {
namespace CLI {
class Subcommand;
class Parser;
}
}

namespace Tool {
struct File;

class AbstractCompiler
{
public:
	virtual ~AbstractCompiler();
	virtual void setup(std::shared_ptr<Util::CLI::Subcommand> &builder) = 0;
	virtual bool run(const Util::CLI::Parser &parser, const File &file) = 0;
	virtual int resolverFlags() const { return 0; }
	virtual std::string name() const = 0;
	virtual std::string help() const = 0;
};
}
}
