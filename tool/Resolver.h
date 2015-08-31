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

#include "DataTypes.h"

namespace Argonauts {
namespace Tool {

class Resolver
{
public:
	explicit Resolver(const File &file);

	class ResolverError : public std::runtime_error
	{
	public:
		explicit ResolverError(const std::string &msg, const int offset_) : std::runtime_error(msg), offset(offset_) {}

		const int offset;
	};

	File result() const { return m_file; }
	void resolveAliases();
	void resolveIncludes();

private:
	File m_file;
};

}
}
