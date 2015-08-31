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
#include <vector>

#include "Parser.h"
#include "Serializer.h"

namespace Argonauts
{
namespace Runtime
{
class Exception : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};
class InvalidEnumValue : public Exception
{
public:
	explicit InvalidEnumValue(const std::string &actual, const std::vector<std::string> &expected);
	explicit InvalidEnumValue(const int actual, const std::vector<int> &expected);
};
class InvalidObjectKey : public Exception
{
public:
	explicit InvalidObjectKey(const std::string &actual, const std::vector<std::string> &expected);
};
}
}
