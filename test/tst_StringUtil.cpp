#include <catch.hpp>

#include "StringUtil.h"

using namespace Argonauts::Util;

TEST_CASE("can convert char to hex string", "[StringUtil]") {
	REQUIRE(String::charToHexString(0x00) == "00");
	REQUIRE(String::charToHexString(0x01) == "01");
	REQUIRE(String::charToHexString(0x10) == "10");
	REQUIRE(String::charToHexString(0xff) == "ff");
}

TEST_CASE("can join strings", "[StringUtil]") {
	REQUIRE(String::joinStrings({"a", "b", "c"}, "") == "abc");
	REQUIRE(String::joinStrings({"a", "b", "c"}, ", ") == "a, b, c");
	REQUIRE(String::joinStrings({"a", "", "c"}, ", ") == "a, , c");
}

TEST_CASE("can split a string", "[StringUtil]") {
	REQUIRE(String::splitStrings("", ";") == std::vector<std::string>());
	REQUIRE(String::splitStrings("a", ";") == std::vector<std::string>({"a"}));
	REQUIRE(String::splitStrings("asdf;bdeaf", ";") == std::vector<std::string>({"asdf", "bdeaf"}));
	REQUIRE(String::splitStrings("asdf;bdeaf;foo", ";") == std::vector<std::string>({"asdf", "bdeaf", "foo"}));
	REQUIRE(String::splitStrings("asdf;bdeaf;;foo;", ";") == std::vector<std::string>({"asdf", "bdeaf", "", "foo", ""}));
}

TEST_CASE("can replace all in strings", "[StringUtil]") {
	REQUIRE(String::replaceAll("this is an test", "an", "a") == "this is a test");
	REQUIRE(String::replaceAll("this is a test", "asdf", "fdsa") == "this is a test");
	REQUIRE(String::replaceAll("abc def", "ab", "ba") == "bac def");
	REQUIRE(String::replaceAll("abc abc abc", "bc", "cb") == "acb acb acb");
	REQUIRE(String::replaceAll("abc abc abc abc", "bc", "sdf") == "asdf asdf asdf asdf");
}

TEST_CASE("can convert string to all uppercase", "[StringUtil]") {
	REQUIRE(String::toUpper("asdf") == "ASDF");
	REQUIRE(String::toUpper("1234") == "1234");
}

TEST_CASE("can take first line", "[StringUtil]") {
	REQUIRE(String::firstLine("this\nis\na\ntest\n") == "this");
	REQUIRE(String::firstLine("\nasdf\n") == "");
}

TEST_CASE("can check startsWith", "[StringUtil]") {
	REQUIRE(String::startsWith("asdf", "as"));
	REQUIRE_FALSE(String::startsWith("asdf", "sd"));
}

TEST_CASE("can check endsWith", "[StringUtil]") {
	REQUIRE(String::endsWith("asdf", "df"));
	REQUIRE_FALSE(String::endsWith("asdf", "sd"));
}
