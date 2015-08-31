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

#include "AbstractCompiler.h"

namespace Argonauts {
namespace Tool {
class DocCompiler : public AbstractCompiler
{
public:
	void setup(std::shared_ptr<Util::CLI::Subcommand> &builder) override;
	bool run(const Util::CLI::Parser &parser, const File &file) override;
	int resolverFlags() const override;
	std::string name() const override { return "doc"; }
	std::string help() const override { return "Generates HTML documentation files"; }

public:
	std::string m_outdir;
};
}
}
