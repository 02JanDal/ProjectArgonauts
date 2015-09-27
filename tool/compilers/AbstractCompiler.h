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

protected:
	template <typename Func, typename... Args>
	void openFileAndCall(const std::string &filename, Func &&func, Args... args)
	{
		openFileAndCallInternal(filename, [func, args...](std::ostream &stream)
		{
			func(stream, args...);
		});
	}

private:
	void openFileAndCallInternal(const std::string &filename, std::function<void(std::ostream&)> &&func);
};
}
}
