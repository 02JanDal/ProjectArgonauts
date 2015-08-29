#include <catch.hpp>

#include "util/json/JsonPointer.h"
#include "util/json/JsonValue.h"

using namespace Argonauts::Util;
using namespace Json;

// examples taken from http://tools.ietf.org/html/draft-ietf-appsawg-json-pointer-04#section-5 and http://tools.ietf.org/html/draft-ietf-appsawg-json-pointer-04#section-6

TEST_CASE("can construct JSON Pointer", "[Json::Pointer]") {
	REQUIRE_FALSE(Pointer().isValid());
	REQUIRE(Pointer("").isValid());
	REQUIRE(Pointer("/foo").isValid());
	REQUIRE(Pointer("/foo/0").isValid());
	REQUIRE(Pointer("/").isValid());
	REQUIRE(Pointer("/a~1b").isValid());
	REQUIRE(Pointer("/c%d").isValid());
	REQUIRE(Pointer("/e^f").isValid());
	REQUIRE(Pointer("/g|h").isValid());
	REQUIRE(Pointer("/i\\j").isValid());
	REQUIRE(Pointer("/k\"j").isValid());
	REQUIRE(Pointer("/ ").isValid());
	REQUIRE(Pointer("/m~0n").isValid());
}

TEST_CASE("can construct JSON Pointer from URI fragment", "[Json::Pointer]") {
	REQUIRE(Pointer("#").isValid());
	REQUIRE(Pointer("#/foo").isValid());
	REQUIRE(Pointer("#/foo/0").isValid());
	REQUIRE(Pointer("#/").isValid());
	REQUIRE(Pointer("#/a~1b").isValid());
	REQUIRE(Pointer("#/c%25d").isValid());
	REQUIRE(Pointer("#/e%5Ef").isValid());
	REQUIRE(Pointer("#/g%7Ch").isValid());
	REQUIRE(Pointer("#/i%5Cj").isValid());
	REQUIRE(Pointer("#/k%22j").isValid());
	REQUIRE(Pointer("#/%20").isValid());
	REQUIRE(Pointer("#/m~0n").isValid());
}

TEST_CASE("can evaluate valid JSON Pointers", "[Json::Pointer]") {
	static const Value value = Value(Object({
									{"foo", Value(Array({Value("bar"), Value("baz")}))},
									{"", Value(0)},
									{"a/b", Value(1)},
									{"c%d", Value(2)},
									{"e^f", Value(3)},
									{"g|h", Value(4)},
									{"i\\j", Value(5)},
									{"k\"l", Value(6)},
									{" ", Value(7)},
									{"m~n", Value(8)}
								}));

	SECTION("normal pointers") {
		REQUIRE(Pointer("").evaluate(value) == value);
		REQUIRE(Pointer("/foo").evaluate(value) == Value(Array({Value("bar"), Value("baz")})));
		REQUIRE(Pointer("/foo/0").evaluate(value) == Value("bar"));
		REQUIRE(Pointer("/").evaluate(value) == Value(0));
		REQUIRE(Pointer("/a~1b").evaluate(value) == Value(1));
		REQUIRE(Pointer("/c%d").evaluate(value) == Value(2));
		REQUIRE(Pointer("/e^f").evaluate(value) == Value(3));
		REQUIRE(Pointer("/g|h").evaluate(value) == Value(4));
		REQUIRE(Pointer("/i\\j").evaluate(value) == Value(5));
		REQUIRE(Pointer("/k\"l").evaluate(value) == Value(6));
		REQUIRE(Pointer("/ ").evaluate(value) == Value(7));
		REQUIRE(Pointer("/m~0n").evaluate(value) == Value(8));
	}

	SECTION("pointers from URI fragments") {
		REQUIRE(Pointer("#").evaluate(value) == value);
		REQUIRE(Pointer("#/foo").evaluate(value) == Value(Array({Value("bar"), Value("baz")})));
		REQUIRE(Pointer("#/foo/0").evaluate(value) == Value("baz"));
		REQUIRE(Pointer("#/").evaluate(value) == Value(0));
		REQUIRE(Pointer("#/a~1b").evaluate(value) == Value(1));
		REQUIRE(Pointer("#/c%25d").evaluate(value) == Value(2));
		REQUIRE(Pointer("#/e%5Ef").evaluate(value) == Value(3));
		REQUIRE(Pointer("#/g%7Ch").evaluate(value) == Value(4));
		REQUIRE(Pointer("#/i%5Cj").evaluate(value) == Value(5));
		REQUIRE(Pointer("#/k%22l").evaluate(value) == Value(6));
		REQUIRE(Pointer("#/%20").evaluate(value) == Value(7));
		REQUIRE(Pointer("#/m~0n").evaluate(value) == Value(8));
	}

	SECTION("throw on invalid pointers") {
		REQUIRE_THROWS_AS(Pointer("/doesnotexist").evaluate(value), Pointer::EvaluationError);
		REQUIRE_THROWS_AS(Pointer("/").evaluate(Value("string")), Pointer::EvaluationError);
		REQUIRE_THROWS_AS(Pointer("/foo/42").evaluate(value), Pointer::EvaluationError);
		REQUIRE_THROWS_AS(Pointer("/foo/not_an_integer").evaluate(value), Pointer::EvaluationError);
	}
}
