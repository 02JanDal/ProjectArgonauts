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

#include "Variant.h"

using namespace Argonauts::Util;

using Type = Variant<char, int64_t, std::string>;

TEST_CASE("can set types", "[Variant]") {
	SECTION("on construction") {
		Type first(char(-1));
		Type second(int64_t(12));
		Type third(std::string("asdf"));
		SECTION("by copy") {
			Type firstCopy(first);
			Type secondCopy(second);
			Type thirdCopy(third);
		}
		SECTION("by move") {
			Type firstMoved(std::move(first));
			Type secondMoved(std::move(second));
			Type thirdMoved(std::move(third));
		}
	}
	SECTION("through assignment") {
		Type first = char(0),
				second = char(0),
				third = char(0);
		first = char(-1);
		second = int64_t(12);
		third = std::string("asdf");
		SECTION("from existing copy") {
			Type firstCopy = char(0),
					secondCopy = char(0),
					thirdCopy = char(0);
			firstCopy = first;
			secondCopy = second;
			thirdCopy = third;
		}
		SECTION("from existing through move") {
			Type firstMoved = char(0),
					secondMoved = char(0),
					thirdMoved = char(0);
			firstMoved = std::move(first);
			secondMoved = std::move(second);
			thirdMoved = std::move(third);
		}
	}
}

TEST_CASE("can compare types", "[Variant]") {
	SECTION("with operator==") {
		REQUIRE(Type(char(-1)) == Type(char(-1)));
		REQUIRE(Type(int64_t(12)) == Type(int64_t(12)));
		REQUIRE(Type(std::string("asdf")) == Type(std::string("asdf")));
		REQUIRE_FALSE(Type(char(-1)) == Type(char(44)));
		REQUIRE_FALSE(Type(char(-1)) == Type(int64_t(-1)));
		REQUIRE_FALSE(Type(char(-1)) == Type(int64_t(44)));
	}
	SECTION("with operator!=") {
		REQUIRE_FALSE(Type(char(-1)) != Type(char(-1)));
		REQUIRE_FALSE(Type(int64_t(12)) != Type(int64_t(12)));
		REQUIRE_FALSE(Type(std::string("asdf")) != Type(std::string("asdf")));
		REQUIRE(Type(char(-1)) != Type(char(44)));
		REQUIRE(Type(char(-1)) != Type(int64_t(-1)));
		REQUIRE(Type(char(-1)) != Type(int64_t(44)));
	}
}

TEST_CASE("can probe type", "[Variant]") {
	REQUIRE(Type(char(-1)).is<char>());
	REQUIRE(Type(int64_t(12)).is<int64_t>());
	REQUIRE(Type(std::string("asdf")).is<std::string>());
	REQUIRE_FALSE(Type(char(-1)).is<int64_t>());
	REQUIRE_FALSE(Type(std::string("asdf")).is<char>());
}

TEST_CASE("can get value back", "[Variant]") {
	SECTION("from non-const Variants") {
		REQUIRE_NOTHROW(Type(char(-1)).get<char>());
		REQUIRE_NOTHROW(Type(int64_t(12)).get<int64_t>());
		REQUIRE_NOTHROW(Type(std::string("asdf")).get<std::string>());
		REQUIRE(Type(char(-1)).get<char>() == -1);
		REQUIRE(Type(int64_t(12)).get<int64_t>() == 12);
		REQUIRE(Type(std::string("asdf")).get<std::string>() == "asdf");
	}
	SECTION("from const Variants") {
		const Type first(char(-1));
		const Type second(int64_t(12));
		const Type third(std::string("asdf"));
		REQUIRE_NOTHROW(first.get<char>());
		REQUIRE_NOTHROW(second.get<int64_t>());
		REQUIRE_NOTHROW(third.get<std::string>());
		REQUIRE(first.get<char>() == -1);
		REQUIRE(second.get<int64_t>() == 12);
		REQUIRE(third.get<std::string>() == "asdf");
	}
	SECTION("but refuses to give a wrong value") {
		SECTION("from non-const Variants") {
			REQUIRE_THROWS(Type(char(-1)).get<int64_t>());
			REQUIRE_THROWS(Type(std::string("asdf")).get<char>());
		}
		SECTION("from const Variants") {
			const Type first(char(-1));
			const Type second(int64_t(12));
			const Type third(std::string("asdf"));
			REQUIRE_THROWS(first.get<int64_t>());
			REQUIRE_THROWS(second.get<std::string>());
		}
	}
}

TEST_CASE("can modify value in-place", "[Variant]") {
	Type first(char(-1));
	Type second(int64_t(12));
	Type third(std::string("asdf"));

	first.get<char>() += 11;
	REQUIRE(first.get<char>() == 10);

	second.get<int64_t>() = 424242;
	REQUIRE(second.get<int64_t>() == 424242);

	third.get<std::string>().append("asdf");
	REQUIRE(third.get<std::string>() == "asdfasdf");
}
