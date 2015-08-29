#include <catch.hpp>

#include "util/json/JsonReference.h"

using namespace Argonauts::Util;
using namespace Json;

TEST_CASE("can construct JSON Reference", "[Json::Reference]") {
	REQUIRE_FALSE(Reference().isValid());
	REQUIRE_FALSE(Reference("").isValid());
	REQUIRE_FALSE(Reference("http://example.com").isValid());
	REQUIRE(Reference("#").isValid());
	REQUIRE(Reference("http://example.com#").isValid());
	REQUIRE(Reference("http://example.com#/foo").isValid());
	REQUIRE(Reference("#/foo").isValid());
}

TEST_CASE("can parse JSON Reference", "[Json::Reference]") {
	REQUIRE(Reference("#").pointer().referenceTokens() == std::vector<std::string>());
	REQUIRE(Reference("#/foo").pointer().referenceTokens() == std::vector<std::string>({"foo"}));
	REQUIRE(Reference("http://example.com#/foo").pointer().referenceTokens() == std::vector<std::string>({"foo"}));
	REQUIRE(Reference("http://example.com#").pointer().referenceTokens() == std::vector<std::string>());
	REQUIRE(Reference("#").uri() == "");
	REQUIRE(Reference("#/foo").uri() == "");
	REQUIRE(Reference("http://example.com#/foo").uri() == "http://example.com");
	REQUIRE(Reference("http://example.com#").uri() == "http://example.com");
}
