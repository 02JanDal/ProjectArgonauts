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

#include "JsonReference.h"

#include "JsonValue.h"
#include "Json.h"

namespace Argonauts {
namespace Util {
namespace Json {
Reference::Reference(const std::string &ref)
	: m_ref(ref)
{
	const std::size_t pound = ref.find('#');
	if (pound != std::string::npos) {
		m_uri = ref.substr(0, pound);
		m_pointer = Pointer(ref.substr(pound + 1));

		m_isValid = (m_uri.empty() || isURI(m_uri)) && m_pointer.isValid();
	}
}
Reference::Reference(const Value &value)
	: Reference(extractReference(value))
{
}
Reference::Reference() {}

std::string Reference::extractReference(const Value &value)
{
	if (value.isString()) {
		return value.toString();
	} else if (value.isObject()) {
		if (value.hasMember("$ref") && value["$ref"].isString()) {
			return value["$ref"].toString();
		}
	}
	return std::string();
}
}
}
}
