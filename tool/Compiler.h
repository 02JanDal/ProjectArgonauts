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
