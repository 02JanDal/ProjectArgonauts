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

namespace Argonauts {
namespace Util {

// Wraps a value, like a string or an integer, with information about where in the input stream it occured
template <typename T>
struct PositionedValue
{
	PositionedValue(const T &value_ = T(), const int offset_ = -1, const int length_ = -1) : value(value_), offset(offset_), length(length_) {}

	T value;
	int offset, length;

	operator T() const { return value; }

	bool operator==(const T &other) const { return value == other; }
	bool operator!=(const T &other) const { return value != other; }
	bool operator==(const PositionedValue<T> &other) const { return value == other.value; }
	bool operator!=(const PositionedValue<T> &other) const { return value == other.value; }
};
using PositionedString = PositionedValue<std::string>;
using PositionedInt64 = PositionedValue<int64_t>;
}
}
template <typename T>
T operator+(const T &a, const Argonauts::Util::PositionedValue<T> &b)
{
	return a + b.value;
}

namespace std {
template <>
struct hash<Argonauts::Util::PositionedString>
{
	std::size_t operator()(const Argonauts::Util::PositionedString &string) const
	{
		return std::hash<std::string>()(string.value);
	}
};
inline std::string to_string(const Argonauts::Util::PositionedString &str) { return str.value; }
}
