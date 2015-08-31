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

#include "JsonValue.h"

#include <cmath>

#include "util/SaxSink.h"
#include "Util.h"

namespace Argonauts {
namespace Util {
namespace Json {
class ParserHandler : public Argonauts::Util::DelegatingSaxSink
{
	Value *m_value;
	std::string m_currentKey;
	bool m_haveHadFirst = false;
public:
	explicit ParserHandler(Value *value) : m_value(value) {}

private:
	template <typename T>
	Value &handle(T &&t)
	{
		if (m_value->isArray()) {
			m_value->toArray().emplace_back(std::forward<T>(t));
			return m_value->toArray().back();
		} else if (m_value->isObject()) {
			m_value->toObject().insert({m_currentKey, Value(std::forward<T>(t))});
			return m_value->toObject().at(m_currentKey);
		} else {
			*m_value = Value(std::forward<T>(t));
			return *m_value;
		}
	}

protected:
	bool nullImpl()
	{
		handle(nullptr);
		return true;
	}
	bool booleanImpl(const bool val)
	{
		handle(val);
		return true;
	}
	bool integerNumberImpl(const int64_t val)
	{
		handle(val);
		return true;
	}
	bool doubleNumberImpl(const double val)
	{
		handle(val);
		return true;
	}
	bool stringImpl(const std::string &str)
	{
		handle(str);
		return true;
	}
	bool startObjectImpl()
	{
		if (m_value->isObject() && !m_haveHadFirst) {
			m_haveHadFirst = true;
			return true; // this "opens" us
		}
		Value &val = handle(Object());
		delegateToObject(new ParserHandler(&val));
		return true;
	}
	bool keyImpl(const std::string &str)
	{
		m_currentKey = str;
		return true;
	}
	bool endObjectImpl(const std::size_t size)
	{
		if (m_value->isObject() && m_value->toObject().size() == size) {
			return true;
		} else {
			reportError("Mismatch between reported and actual object size");
			return false;
		}
	}
	bool startArrayImpl()
	{
		if (m_value->isArray() && !m_haveHadFirst) {
			m_haveHadFirst = true;
			return true; // this "opens" us
		}
		Value &val = handle(Array());
		delegateToArray(new ParserHandler(&val));
		return true;
	}
	bool endArrayImpl(const std::size_t size)
	{
		if (m_value->isArray() && m_value->toArray().size() == size) {
			return true;
		} else {
			reportError("Mismatch between reported and actual array size");
			return false;
		}
	}
};

void Value::clear()
{
	m_string.empty();
	m_array.clear();
	m_object.clear();
}

//Value::Value(const Value &other)
//	: m_type(other.m_type)
//{
//	switch (m_type) {
//	case Type::Null: break;
//	case Type::Boolean:
//		m_boolean = other.m_boolean; break;
//	case Type::Integer:
//		m_integer = other.m_integer; break;
//	case Type::Number:
//		m_double = other.m_double; break;
//	case Type::String:
//		m_string = other.m_string; break;
//	case Type::Array:
//		m_array = other.m_array; break;
//	case Type::Object:
//		m_object = other.m_object; break;
//	case Type::Invalid:
//		break;
//	}
//}

void Value::operator=(const ObjectContainerT<Value> &object)
{
	clear();
	m_object = object;
	m_type = Type::Object;
}
void Value::operator=(const ArrayContainerT<Value> &array)
{
	clear();
	m_array = array;
	m_type = Type::Array;
}
void Value::operator=(const StringT &string)
{
	clear();
	m_string = string;
	m_type = Type::String;
}
void Value::operator=(const DoubleT &double_)
{
	clear();
	m_double = double_;
	m_type = Type::Number;
}
void Value::operator=(const IntegerT &integer)
{
	clear();
	m_integer = integer;
	m_type = Type::Integer;
}
void Value::operator=(const BooleanT &boolean)
{
	clear();
	m_boolean = boolean;
	m_type = Type::Boolean;
}

Type Value::type() const
{
	if (isArray()) {
		return Type::Array;
	} else if (isObject()) {
		return Type::Object;
	} else if (isBoolean()) {
		return Type::Boolean;
	} else if (isInteger() || isDouble()) {
		return Type::Number;
	} else if (isNull()) {
		return Type::Null;
	} else if (isString()) {
		return Type::String;
	} else {
		ASSERT(false);
		return Type::Null;
	}
}

std::size_t Value::size() const
{
	ASSERT(isObject() || isArray() || isString())
	if (isObject()) {
		return m_object.size();
	} else if (isArray()) {
		return m_array.size();
	} else {
		return m_string.empty();
	}
}

SaxSink *Value::parserSink(Value *value)
{
	return new ParserHandler(value);
}

static bool fuzzyCompare(const double a, const double b)
{
	return std::abs(a - b) * 1000000000000. <= std::min(std::abs(a), std::abs(b));
}

bool Value::operator==(const Value &other) const
{
	if (m_type != other.m_type) {
		return false;
	}
	if (isString()) {
		return toString() == other.toString();
	} else if (isBoolean()) {
		return toBoolean() == other.toBoolean();
	} else if (isDouble()) {
		return fuzzyCompare(toDouble(), other.toDouble());
	} else if (isInteger()) {
		return toInteger() == other.toInteger();
	} else if (isArray()) {
		return toArray() == other.toArray();
	} else if (isObject()) {
		return toObject() == other.toObject();
	} else if (isNull()) {
		return true;
	} else {
		return false;
	}
}
}
}
}
