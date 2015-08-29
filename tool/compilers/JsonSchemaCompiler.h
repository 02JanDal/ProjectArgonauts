#pragma once

#include "AbstractCompiler.h"

namespace Argonauts {
namespace Tool {
class JsonSchemaCompiler : public AbstractCompiler
{
public:
	void setup(std::shared_ptr<Util::CLI::Subcommand> &builder) override;
	bool run(const Util::CLI::Parser &parser, const File &file) override;
	std::string name() const override { return "jsonschema"; }
	std::string help() const override { return "Generates JSON Schema files"; }
};
}
}
