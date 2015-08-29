#include <catch.hpp>

#include "SaxSink.h"
#include "json/JsonSaxWriter.h"
#include "json/JsonSaxReader.h"
#include "Util.h"

using namespace Argonauts::Util;
using namespace Json;

class TestingJsonSaxHandler : public SaxSink
{
public:
	struct Item
	{
		enum Type
		{
			Null, Boolean, Integer, Double, String, StartObject, Key, EndObject, StartArray, EndArray
		};
		const Type type;
		const bool boolean = true;
		const int64_t integer = 0;
		const double double_ = 0;
		const std::string string;
		const std::size_t size = 0;

		explicit Item(const Type type) : type(type) {}
		explicit Item(const bool boolean) : type(Boolean), boolean(boolean) {}
		explicit Item(const int64_t integer) : type(Integer), integer(integer) {}
		explicit Item(const double double_) : type(Double), double_(double_) {}
		explicit Item(const Type type, const std::string &string) : type(type), string(string) {}
		explicit Item(const Type type, const std::size_t &size) : type(type), size(size) {}

		bool operator==(const Item &other) const
		{
			if (type != other.type)
			{
				return false;
			}
			switch (type)
			{
			case Null:
			case StartObject:
			case StartArray:
				return true;
			case Boolean: return boolean == other.boolean;
			case Integer: return integer == other.integer;
			case Double: return double_ == other.double_;
			case String:
			case Key:
				return string == other.string;
			case EndObject:
			case EndArray:
				return size == other.size;
			default:
				return false;
			}
		}
	};
	std::vector<Item> m_items;

	bool null()
	{
		m_items.emplace_back(Item::Null);
		return true;
	}
	bool boolean(const bool val)
	{
		m_items.emplace_back(val);
		return true;
	}
	bool integerNumber(const int64_t val)
	{
		m_items.emplace_back(val);
		return true;
	}
	bool doubleNumber(const double val)
	{
		m_items.emplace_back(val);
		return true;
	}
	bool string(const std::string &str)
	{
		m_items.emplace_back(Item::String, str);
		return true;
	}
	bool startObject()
	{
		m_items.emplace_back(Item::StartObject);
		return true;
	}
	bool key(const std::string &str)
	{
		m_items.emplace_back(Item::Key, str);
		return true;
	}
	bool endObject(const std::size_t size)
	{
		m_items.emplace_back(Item::EndObject, size);
		return true;
	}
	bool startArray()
	{
		m_items.emplace_back(Item::StartArray);
		return true;
	}
	bool endArray(const std::size_t size)
	{
		m_items.emplace_back(Item::EndArray, size);
		return true;
	}
};

const std::string jsonData1 = R"json(
{
	"string": "this is a test",
	"integerPositive": 12345,
	"integerNegative": -12345,
	"double": 42.42,
	"object": {
		"keywithspecialchars!#\"+-": "stringwith specialchars!#\"+-",
		"object": {}
	},
	"array": [
		"string",
		24.24,
		[[[[]]]]
	]
}
)json";
const std::string jsonData2 = "{\"string\":\"this is a test\",\"integerPositive\":12345,\"integerNegative\":-12345,\"double\":42.42,\"object\":{\"keywithspecialchars!#\\\"+-\":\"stringwith specialchars!#\\\"+-\",\"object\":{}},\"array\":[\"string\",24.24,[[[[]]]]]}";

TEST_CASE("can parse strict JSON in one go", "[Json::SaxReader]") {
	SECTION("with object as root") {
		TestingJsonSaxHandler *handler = new TestingJsonSaxHandler;
		SaxReader parser(handler);
		parser.addData(jsonData1);
		parser.end();
		if (parser.isError()) {
			INFO(parser.error(jsonData1).errorMessage());
		}
		REQUIRE_FALSE(parser.isError());
		REQUIRE(handler->m_items.size() == 31);
	}
}

TEST_CASE("can do roundtrip", "[Json::SaxReader][Json::SaxWriter][StringOutputStream]") {
	StringOutputStream output;
	SaxWriter handler(&output);
	SaxReader parser(&handler);
	parser.addData(jsonData2);
	parser.end();
	if (parser.isError()) {
		INFO(parser.error(jsonData1).errorMessage());
	}
	REQUIRE_FALSE(parser.isError());
	REQUIRE(output.result() == jsonData2);
}
