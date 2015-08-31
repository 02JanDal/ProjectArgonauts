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

#include "Argonauts.h"

#include <algorithm>

#include "util/StringUtil.h"

namespace Argonauts {
namespace Runtime {
namespace detail {
static std::vector<std::string> intsToStringsHelper(const std::vector<int> &in)
{
	std::vector<std::string> out;
	std::transform(in.begin(), in.end(), std::back_inserter(out), [](const int val) { return std::to_string(val); });
	return out;
}
}

InvalidEnumValue::InvalidEnumValue(const std::string &actual, const std::vector<std::string> &expected)
	: Exception(std::string("Unexpected enum value '") + actual + "', expected one of: '" + Util::String::joinStrings(expected, "', '") + "'") {}
InvalidEnumValue::InvalidEnumValue(const int actual, const std::vector<int> &expected)
	: InvalidEnumValue(std::to_string(actual), detail::intsToStringsHelper(expected)) {}

InvalidObjectKey::InvalidObjectKey(const std::string &actual, const std::vector<std::string> &expected)
	: Exception(std::string("Unexpected object key '") + actual + "', expected one of: '" + Util::String::joinStrings(expected, "', '") + "'") {}

}
}
