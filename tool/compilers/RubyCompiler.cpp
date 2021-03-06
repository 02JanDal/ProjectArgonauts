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

#include "RubyCompiler.h"

#include "util/CmdParser.h"
#include "tool/DataTypes.h"

namespace Argonauts {
namespace Tool {
void RubyCompiler::setup(std::shared_ptr<Util::CLI::Subcommand> &builder)
{
	builder->addOption({"ext"}, "If specified, makes use of Ruby native extensions")->applyTo(this, &RubyCompiler::m_nativeExtensions);
}

bool RubyCompiler::run(const Util::CLI::Parser &parser, const File &file)
{
	return false;
}
}
}
