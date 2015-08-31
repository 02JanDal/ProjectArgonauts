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

#include <catch.hpp>

#include "util/json/JsonValue.h"
#include "util/json/JsonSaxReader.h"

using namespace Argonauts::Util;
using namespace Json;

TEST_CASE("basic accessors work", "[Json::Value]") {
	REQUIRE(Value(std::string()).isString());
	REQUIRE(Value(1.0f).isDouble());
	REQUIRE(Value(int64_t(42)).isInteger());
	REQUIRE(Value(Array()).isArray());
	REQUIRE(Value(Object()).isObject());
	REQUIRE(Value(nullptr).isNull());

	REQUIRE(Value(std::string()).type() == Type::String);
	REQUIRE(Value(1.0f).type() == Type::Number);
	REQUIRE(Value(int64_t(42)).type() == Type::Number);
	REQUIRE(Value(Array()).type() == Type::Array);
	REQUIRE(Value(Object()).type() == Type::Object);
	REQUIRE(Value(nullptr).type() == Type::Null);

	REQUIRE(Value(std::string("asdf")).toString() == "asdf");
	REQUIRE(Value(1.0f).toDouble() == 1.0f);
	REQUIRE(Value(int64_t(42)).toInteger() == 42);
	REQUIRE(Value(Array()).toArray() == Array());
	REQUIRE(Value(Object()).toObject() == Object());
}

const std::string jsonData = "{\"string\":\"this is a test\",\"integerPositive\":12345,\"integerNegative\":-12345,\"double\":42.42,\"object\":{\"keywithspecialchars!#\\\"+-\":\"stringwith specialchars!#\\\"+-\",\"object\":{}},\"array\":[\"string\",24.24,[[[[]]]]]}";
TEST_CASE("DOM parser works", "[Json::Value][Json::SaxReader]") {
	Value val;
	SaxReader reader(Value::parserSink(&val));
	reader.addData(jsonData);
	reader.end();
	REQUIRE_FALSE(reader.isError());
	REQUIRE(val.isObject());
	REQUIRE(val.toObject().size() == 6);
	REQUIRE(val["string"].toString() == "this is a test");
	REQUIRE(val["integerPositive"].toInteger() == 12345);
	REQUIRE(val["integerNegative"].toInteger() == -12345);
	REQUIRE(val["double"].toDouble() == 42.42);
	REQUIRE(val["object"]["keywithspecialchars!#\"+-"].toString() == "stringwith specialchars!#\"+-");
	REQUIRE(val["array"].isArray());
	REQUIRE(val["array"][2].isArray());
	REQUIRE(val["array"][2][0].isArray());
	REQUIRE(val["array"][2][0][0].isArray());
	REQUIRE(val["array"][2][0][0][0].isArray());
}
