#include <catch.hpp>

#include "StringUtil.h"

TEST_CASE("can convert char to hex string", "[StringUtil]") {
	REQUIRE(StringUtil::charToHexString(0x00) == "00");
	REQUIRE(StringUtil::charToHexString(0x01) == "01");
	REQUIRE(StringUtil::charToHexString(0x10) == "10");
	REQUIRE(StringUtil::charToHexString(0xff) == "ff");
}

TEST_CASE("can join strings", "[StringUtil]") {
	REQUIRE(StringUtil::joinStrings({"a", "b", "c"}, "") == "abc");
	REQUIRE(StringUtil::joinStrings({"a", "b", "c"}, ", ") == "a, b, c");
	REQUIRE(StringUtil::joinStrings({"a", "", "c"}, ", ") == "a, , c");
}

TEST_CASE("can split a string", "[StringUtil]") {
	REQUIRE(StringUtil::splitStrings("", ";") == std::vector<std::string>());
	REQUIRE(StringUtil::splitStrings("a", ";") == std::vector<std::string>({"a"}));
	REQUIRE(StringUtil::splitStrings("asdf;bdeaf", ";") == std::vector<std::string>({"asdf", "bdeaf"}));
	REQUIRE(StringUtil::splitStrings("asdf;bdeaf;foo", ";") == std::vector<std::string>({"asdf", "bdeaf", "foo"}));
	REQUIRE(StringUtil::splitStrings("asdf;bdeaf;;foo;", ";") == std::vector<std::string>({"asdf", "bdeaf", "", "foo", ""}));
}

TEST_CASE("can replace all in strings", "[StringUtil]") {
	REQUIRE(StringUtil::replaceAll("this is an test", "an", "a") == "this is a test");
	REQUIRE(StringUtil::replaceAll("this is a test", "asdf", "fdsa") == "this is a test");
	REQUIRE(StringUtil::replaceAll("abc def", "ab", "ba") == "bac def");
	REQUIRE(StringUtil::replaceAll("abc abc abc", "bc", "cb") == "acb acb acb");
	REQUIRE(StringUtil::replaceAll("abc abc abc abc", "bc", "sdf") == "asdf asdf asdf asdf");
}

TEST_CASE("can convert string to all uppercase", "[StringUtil]") {
	REQUIRE(StringUtil::toUpper("asdf") == "ASDF");
	REQUIRE(StringUtil::toUpper("1234") == "1234");
}

TEST_CASE("can take first line", "[StringUtil]") {
	REQUIRE(StringUtil::firstLine("this\nis\na\ntest\n") == "this");
	REQUIRE(StringUtil::firstLine("\nasdf\n") == "");
}

TEST_CASE("can check startsWith", "[StringUtil]") {
	REQUIRE(StringUtil::startsWith("asdf", "as"));
	REQUIRE_FALSE(StringUtil::startsWith("asdf", "sd"));
}

TEST_CASE("can check endsWith", "[StringUtil]") {
	REQUIRE(StringUtil::endsWith("asdf", "df"));
	REQUIRE_FALSE(StringUtil::endsWith("asdf", "sd"));
}
