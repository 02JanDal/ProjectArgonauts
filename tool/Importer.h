#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Argonauts {
namespace Util {
namespace Json {
using SchemaPtr = std::shared_ptr<class Schema>;
class SchemaResolver;
}
namespace CLI {
class Parser;
class ParserBuilder;
}
}

namespace Tool {
class Importer
{
public:
	explicit Importer();

	std::vector<std::string> schemas;
	std::string output;
	bool force = false;

	void setup(std::shared_ptr<Util::CLI::ParserBuilder> &builder);
	bool run(const Util::CLI::Parser &parser);

private:
	void dump(std::ostream &stream, const Util::CLI::Parser &parser, const Util::Json::SchemaResolver &resolver);
	void dumpSchema(std::ostream &stream, const std::string &filename, const Util::Json::SchemaPtr &schema);
};
}
}
