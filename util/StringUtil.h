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
#include <vector>

using StringVector = std::vector<std::string>;

namespace Argonauts {
namespace Util {
namespace String {
std::string charToHexString(const char c);
std::string joinStrings(const StringVector &strings, const std::string &glue);
StringVector splitStrings(const std::string &string, const std::string &delimiter);
std::string replaceAll(const std::string &in, const std::string &match, const std::string &replacement);
std::string toUpper(const std::string &in);
std::string firstLine(const std::string &in);
bool startsWith(const std::string &str, const std::string &match);
bool endsWith(const std::string &str, const std::string &match);
}
}
}
