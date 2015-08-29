#pragma once

#include "AbstractCompiler.h"

namespace Argonauts {
namespace Tool {
class CppCompiler : public AbstractCompiler
{
public:
	void setup(std::shared_ptr<Util::CLI::Subcommand> &builder) override;
	bool run(const Util::CLI::Parser &parser, const File &file) override;
	std::string name() const override { return "cpp"; }
	std::string help() const override { return "Generates C++ files"; }

private:
	std::string m_dataTypes = "stl";
	std::string m_directory;

	std::string filenameFor(const std::string &type, const bool header) const;
};
}
}
