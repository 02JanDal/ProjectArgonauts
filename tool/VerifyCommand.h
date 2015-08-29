#pragma once

#include <string>
#include <vector>

#include "util/CmdParser.h"

namespace Argonauts {
namespace Tool {

class VerifyCommand
{
public:
	explicit VerifyCommand();

	void setup(Util::CLI::ParserBuilder::Ptr &builder);
	Util::CLI::Execution run(const Util::CLI::Parser &parser);

private:
};
}
}
