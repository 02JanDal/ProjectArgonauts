/*
 * Copyright 2015 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
