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

#include "FSUtil.h"

#include <fstream>
#include <istream>
#include <ostream>

#include "ArgonautsException.h"

namespace Argonauts {
namespace Util {
namespace FS {

std::string readFile(const std::string &filename)
{
	std::ifstream stream;
	stream.open(filename);
	if (!stream.is_open() || !stream.good()) {
		throw Exception(std::string("Unable to open file '") + filename + "' for reading");
	}
	stream.seekg(0, std::ios_base::end);
	const auto length = stream.tellg();
	if (length == -1) {
		throw Exception(std::string("Unable to open file '") + filename + "' for reading");
	}
	stream.seekg(0, std::ios_base::beg);
	char *buf = new char[std::size_t(length)];
	stream.read(buf, length);
	stream.close();
	return std::string(buf, std::size_t(length));
}

void writeFile(const std::string &filename, const std::string &data)
{
	std::ofstream stream;
	stream.open(filename);
	if (!stream.is_open() || !stream.good()) {
		throw Exception(std::string("Unable to open file '") + filename + "' for writing");
	}
	stream << data;
	stream.close();
}

}
}
}
