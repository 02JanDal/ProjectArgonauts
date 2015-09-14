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
#include <map>

#include "util/Util.h"

namespace Argonauts {
namespace Util {
class SaxSink;

namespace Json {
// json primitives, see below for Array and Object
using NullT = std::nullptr_t;
using BooleanT = bool;
using IntegerT = int64_t;
using DoubleT = double;
using StringT = std::string;
template <typename T>
using ArrayContainerT = std::vector<T>;
template <typename T>
using ObjectContainerT = std::map<StringT, T>;

enum class Type
{
	Invalid, Array, Boolean, Null, Number, Integer, Object, String
};

class Value
{
	// TODO: make these use the same memory
	NullT m_null;
	BooleanT m_boolean;
	IntegerT m_integer;
	DoubleT m_double;
	StringT m_string;
	ArrayContainerT<Value> m_array;
	ObjectContainerT<Value> m_object;

	Type m_type;

	void clear();

public:
	explicit Value(const std::nullptr_t &null = nullptr) : m_null(null), m_type(Type::Null) {}
	explicit Value(const BooleanT &boolean) : m_boolean(boolean), m_type(Type::Boolean) {}
	explicit Value(const IntegerT &integer) : m_integer(integer), m_type(Type::Integer) {}
	explicit Value(const DoubleT &double_) : m_double(double_), m_type(Type::Number) {}
	explicit Value(const StringT &string) : m_string(string), m_type(Type::String) {}
	explicit Value(const ArrayContainerT<Value> &array) : m_array(array), m_type(Type::Array) {}
	explicit Value(const ObjectContainerT<Value> &object) : m_object(object), m_type(Type::Object) {}

	// extras:
	explicit Value(const int integer) : Value(int64_t(integer)) {}
	explicit Value(char *str) : Value(std::string(str)) {}
	explicit Value(const char *str) : Value(std::string(str)) {}

	//Value(const Value &other);

	void operator=(const BooleanT &boolean);
	void operator=(const IntegerT &integer);
	void operator=(const DoubleT &double_);
	void operator=(const StringT &string);
	void operator=(const ArrayContainerT<Value> &array);
	void operator=(const ObjectContainerT<Value> &object);

	bool isNull() const { return m_type == Type::Null; }
	bool isBoolean() const { return m_type == Type::Boolean; }
	bool isInteger() const { return m_type == Type::Integer; }
	bool isDouble() const { return m_type == Type::Number; }
	bool isString() const { return m_type == Type::String; }
	bool isArray() const { return m_type == Type::Array; }
	bool isObject() const { return m_type == Type::Object; }

	Type type() const;

	BooleanT &toBoolean() { ASSERT(isBoolean()); return m_boolean; }
	const BooleanT &toBoolean() const { ASSERT(isBoolean()); return m_boolean; }
	IntegerT &toInteger() { ASSERT(isInteger()); return m_integer; }
	const IntegerT &toInteger() const { ASSERT(isInteger()); return m_integer; }
	DoubleT &toDouble() { ASSERT(isDouble()); return m_double; }
	const DoubleT &toDouble() const { ASSERT(isDouble()); return m_double; }
	StringT &toString() { ASSERT(isString()); return m_string; }
	const StringT &toString() const { ASSERT(isString()); return m_string; }
	ArrayContainerT<Value> &toArray() { ASSERT(isArray()); return m_array; }
	const ArrayContainerT<Value> &toArray() const { ASSERT(isArray()); return m_array; }
	ObjectContainerT<Value> &toObject() { ASSERT(isObject()); return m_object; }
	const ObjectContainerT<Value> &toObject() const { ASSERT(isObject()); return m_object; }

	bool hasMember(const std::string &key) const { return toObject().count(key) > 0; }
	std::size_t size() const;

	Value &operator[](const std::string &key) { return toObject().at(key); }
	const Value &operator[](const std::string &key) const { return toObject().at(key); }
	Value &operator[](const std::size_t index) { return toArray()[index]; }
	const Value &operator[](const std::size_t index) const { return toArray()[index]; }

	static SaxSink *parserSink(Value *value);

	bool operator==(const Value &other) const;
	bool operator!=(const Value &other) const;
};

using Array = ArrayContainerT<Value>;
using Object = ObjectContainerT<Value>;
}
}
}
