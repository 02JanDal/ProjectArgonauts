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

#include "JsonPointer.h"

namespace Argonauts {
namespace Util {
namespace Json {
class Value;

/// Parses and handles a JSON Reference (http://tools.ietf.org/html/draft-pbryan-zyp-json-ref-03)
class Reference
{
	bool m_isValid = false;
	std::string m_ref;
	std::string m_uri;
	Pointer m_pointer;
public:
	explicit Reference(const std::string &ref);
	explicit Reference(const Value &value);
	explicit Reference();

	bool isValid() const { return m_isValid; }
	std::string uri() const { return m_uri; }
	Pointer pointer() const { return m_pointer; }

private:
	std::string extractReference(const Value &value);
};
}
}
}
