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

#include <stdexcept>
#include <string>

namespace Argonauts {
namespace Util {

class Error : public std::runtime_error
{
public:
	class Source
	{
		friend class Error;
		const std::string m_data;
		const std::string m_filename;
	public:
		explicit Source(const std::string &data, const std::string &filename = "<unknown>");
		explicit Source();
	};

	explicit Error();
	explicit Error(const std::string &msg, const std::size_t offset = std::string::npos, const Source &source = Source());

	std::pair<long, long> lineColumnFromDataAndOffset() const;
	std::string lineInData(const int offset = 0) const;
	std::string errorMessage() const;

private:
	const std::string m_error;
	const std::size_t m_offset = std::string::npos;
	const Source m_source;
};

}
}
