#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "util/Util.h"
#include "util/SaxSink.h"
#include "util/Variant.h"
#include "JsonValue.h"

namespace Argonauts {
namespace Util {
namespace Json {
using SchemaPtr = std::shared_ptr<class Schema>;

class Schema
{
public:
	~Schema() {}

	StringT ref; // format: JSONReference
	StringT schema; // format: uri

	StringT id;
	StringT title;
	StringT description;
	Value default_;
	IntegerT multipleOf = -1; // minimum: 0, exclusiveMinimum: true
	IntegerT maximum = -1;
	BooleanT exclusiveMaximum = false; // default: false
	IntegerT minimum = -1;
	BooleanT exclusiveMinimum = false; // default: false
	IntegerT maxLength = -1; // minimum: 0
	IntegerT minLength = 0; // minimum: 0, default: 0
	StringT pattern; // format: regex
	StringT format;
	Variant<NullT, SchemaPtr, bool> additionalItems = nullptr; // default: {}
	Variant<NullT, SchemaPtr, ArrayContainerT<SchemaPtr>> items = nullptr; // default: {}, two::minItems: 1
	IntegerT maxItems = -1; // minimum: 0
	IntegerT minItems = 0; // minimum: 0, default: 0
	BooleanT uniqueItems = false; // default: false
	IntegerT maxProperties = -1; // minimum: 0
	IntegerT minProperties = 0; // minimum: 0, default: 0
	ArrayContainerT<StringT> required; // minItems: 1, uniqueItems: true
	Variant<NullT, SchemaPtr, bool> additionalProperties = nullptr; // default: {}
	ObjectContainerT<SchemaPtr> definitions; // default: {}
	ObjectContainerT<SchemaPtr> properties; // default: {}
	ObjectContainerT<SchemaPtr> patternProperties; // default: {}
	ObjectContainerT<Variant<SchemaPtr, ArrayContainerT<StringT>>> dependencies; // value::two::minItems: 1
	ArrayContainerT<StringT> enum_; // minItems: 1, uniqueItems: true
	Variant<NullT, Type, ArrayContainerT<Type>> type = nullptr; // two::minItems: 1, two::uniqueItems: true
	ArrayContainerT<SchemaPtr> allOf; // minItems: 1
	ArrayContainerT<SchemaPtr> anyOf; // minItems: 1
	ArrayContainerT<SchemaPtr> oneOf; // minItems: 1
	SchemaPtr not_;
};

class SchemaParserHandler : public DelegatingSaxSink
{
	SchemaPtr m_schema;
	std::string m_currentKey;
public:
	explicit SchemaParserHandler(SchemaPtr schema);

private:
	bool nullImpl() override;
	bool booleanImpl(const bool val) override;
	bool integerNumberImpl(const int64_t val) override;
	bool doubleNumberImpl(const double val) override;
	bool stringImpl(const std::string &str) override;
	bool startObjectImpl() override;
	bool keyImpl(const std::string &str) override;
	bool endObjectImpl(const std::size_t size) override;
	bool startArrayImpl() override;
	bool endArrayImpl(const std::size_t size) override;
};
}
}
}
