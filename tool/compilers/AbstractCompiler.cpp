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

#include "AbstractCompiler.h"

#include <fstream>
#include <ostream>

#include <boost/filesystem.hpp>

#include "util/ArgonautsException.h"

namespace Argonauts {
namespace Tool {
AbstractCompiler::~AbstractCompiler() {}

void AbstractCompiler::openFileAndCallInternal(const std::string &filename, std::function<void(std::ostream&)> &&func)
{
	using namespace boost;

	filesystem::path path = filesystem::path(filename).parent_path();
	if (!filesystem::exists(path)) {
		if (!filesystem::create_directories(path)) {
			throw Util::Exception(std::string("Unable to create parent directories for ") + filename);
		}
	}
	if (filesystem::exists(filesystem::path(filename)) && filesystem::is_regular_file(path)) {
		throw Util::Exception(std::string("'") + filename + "' already exists but is not a file");
	}

	std::ofstream stream(filename);
	if (!stream.good()) {
		throw Util::Exception(std::string("Unable to open file '") + filename + "' for writing");
	}
	func(stream);
}

}
}
