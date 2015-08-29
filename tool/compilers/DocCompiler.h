#pragma once

#include "AbstractCompiler.h"

namespace Argonauts {
namespace Tool {
class DocCompiler : public AbstractCompiler
{
public:
	void setup(std::shared_ptr<Util::CLI::Subcommand> &builder) override;
	bool run(const Util::CLI::Parser &parser, const File &file) override;
	std::string name() const override { return "doc"; }
	std::string help() const override { return "Generates HTML documentation files"; }

public:
	std::string m_outdir;
};
}
}
