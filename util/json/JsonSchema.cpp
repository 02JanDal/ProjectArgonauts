#include "JsonSchema.h"

#include "Json.h"

namespace Argonauts {
namespace Util {
namespace Json {
class StringArrayParser : public SaxSink
{
	ArrayContainerT<StringT> *m_container;
public:
	explicit StringArrayParser(ArrayContainerT<StringT> *container)
		: m_container(container) {}

protected:
	bool null() override
	{
		return reportError("Invalid value type 'null', expected 'string'");
	}
	bool boolean(const bool) override
	{
		return reportError("Invalid value type 'boolean', expected 'string'");
	}
	bool integerNumber(const int64_t) override
	{
		return reportError("Invalid value type 'number', expected 'string'");
	}
	bool doubleNumber(const double) override
	{
		return reportError("Invalid value type 'number', expected 'string'");
	}
	bool string(const std::string &str) override
	{
		m_container->push_back(str);
		return true;
	}
	bool startObject() override
	{
		return reportError("Invalid value type 'object', expected 'string'");
	}
	bool key(const std::string &) override
	{
		return false;
	}
	bool endObject(const std::size_t) override
	{
		return false;
	}
	bool startArray() override
	{
		return reportError("Invalid value type 'array', expected 'string'");
	}
	bool endArray(const std::size_t) override
	{
		return true;
	}
};
class TypeArrayParser : public SaxSink
{
	ArrayContainerT<Type> *m_container;
public:
	explicit TypeArrayParser(ArrayContainerT<Type> *container)
		: m_container(container) {}

	static Type typeFromString(const std::string &str)
	{
		if (str == "array") {
			return Type::Array;
		} else if (str == "boolean") {
			return Type::Boolean;
		} else if (str == "number") {
			return Type::Number;
		} else if (str == "integer") {
			return Type::Integer;
		} else if (str == "null") {
			return Type::Null;
		} else if (str == "object") {
			return Type::Object;
		} else if (str == "string") {
			return Type::String;
		} else {
			return Type::Invalid;
		}
	}

protected:
	bool null() override
	{
		return reportError("Invalid value type 'null', expected 'string'");
	}
	bool boolean(const bool) override
	{
		return reportError("Invalid value type 'boolean', expected 'string'");
	}
	bool integerNumber(const int64_t) override
	{
		return reportError("Invalid value type 'number', expected 'string'");
	}
	bool doubleNumber(const double) override
	{
		return reportError("Invalid value type 'number', expected 'string'");
	}
	bool string(const std::string &str) override
	{
		Type type = typeFromString(str);
		if (type == Type::Invalid) {
			return reportError("Invalid string value '%s', expected one of 'array', 'boolean', 'number', 'null', 'object', 'string'", str.c_str());
		} else {
			m_container->push_back(type);
		}
		return true;
	}
	bool startObject() override
	{
		return reportError("Invalid value type 'object', expected 'string'");
	}
	bool key(const std::string &) override
	{
		return false;
	}
	bool endObject(const std::size_t) override
	{
		return false;
	}
	bool startArray() override
	{
		return reportError("Invalid value type 'array', expected 'string'");
	}
	bool endArray(const std::size_t) override
	{
		return true;
	}
};
class SchemaMapParser : public DelegatingSaxSink
{
	ObjectContainerT<SchemaPtr> *m_map;
	StringT m_currentKey;
public:
	explicit SchemaMapParser(ObjectContainerT<SchemaPtr> *map)
		: m_map(map) {}

protected:
	bool nullImpl() override
	{
		return reportError("Invalid value type 'null', expected 'object'");
	}
	bool booleanImpl(const bool) override
	{
		return reportError("Invalid value type 'boolean', expected 'object'");
	}
	bool integerNumberImpl(const int64_t) override
	{
		return reportError("Invalid value type 'number', expected 'object'");
	}
	bool doubleNumberImpl(const double) override
	{
		return reportError("Invalid value type 'number', expected 'object'");
	}
	bool stringImpl(const std::string &) override
	{
		return reportError("Invalid value type 'string', expected 'object'");
	}
	bool startObjectImpl() override
	{
		if (m_currentKey.empty()) {
			return true; // this "opens" us
		}
		(*m_map)[m_currentKey] = std::make_shared<Schema>();
		delegateToObject(new SchemaParserHandler((*m_map)[m_currentKey]));
		return true;
	}
	bool keyImpl(const std::string &str) override
	{
		m_currentKey = str;
		return true;
	}
	bool endObjectImpl(const std::size_t) override
	{
		return true;
	}
	bool startArrayImpl() override
	{
		return reportError("Invalid value type 'array', expected 'object'");
	}
	bool endArrayImpl(const std::size_t) override
	{
		return false;
	}
};
class SchemaArrayParser : public DelegatingSaxSink
{
	ArrayContainerT<SchemaPtr> *m_container;
	bool m_wasStarted = true;
public:
	explicit SchemaArrayParser(ArrayContainerT<SchemaPtr> *container)
		: m_container(container) {}

protected:
	bool nullImpl() override
	{
		return reportError("Invalid value type 'null', expected 'object'");
	}
	bool booleanImpl(const bool) override
	{
		return reportError("Invalid value type 'boolean', expected 'object'");
	}
	bool integerNumberImpl(const int64_t) override
	{
		return reportError("Invalid value type 'number', expected 'object'");
	}
	bool doubleNumberImpl(const double) override
	{
		return reportError("Invalid value type 'number', expected 'object'");
	}
	bool stringImpl(const std::string &) override
	{
		return reportError("Invalid value type 'string', expected 'object'");
	}
	bool startObjectImpl() override
	{
		m_container->push_back(std::make_shared<Schema>());
		delegateToObject(new SchemaParserHandler(m_container->back()));
		return true;
	}
	bool keyImpl(const std::string &) override
	{
		return false;
	}
	bool endObjectImpl(const std::size_t) override
	{
		return false;
	}
	bool startArrayImpl() override
	{
		if (!m_wasStarted) {
			m_wasStarted = true;
			return true;
		}
		return reportError("Invalid value type 'array', expected 'object'");
	}
	bool endArrayImpl(const std::size_t) override
	{
		return true;
	}
};

class DependenciesParser : public DelegatingSaxSink
{
	ObjectContainerT<Variant<SchemaPtr, ArrayContainerT<StringT>>> *m_container;
	std::string m_currentKey;
public:
	explicit DependenciesParser(ObjectContainerT<Variant<SchemaPtr, ArrayContainerT<StringT>>> *container)
		: m_container(container) {}

protected:
	bool nullImpl() override
	{
		return reportError("Invalid value type 'null', expected 'object' or 'array'");
	}
	bool booleanImpl(const bool) override
	{
		return reportError("Invalid value type 'null', expected 'object' or 'array'");
	}
	bool integerNumberImpl(const int64_t) override
	{
		return reportError("Invalid value type 'null', expected 'object' or 'array'");
	}
	bool doubleNumberImpl(const double) override
	{
		return reportError("Invalid value type 'null', expected 'object' or 'array'");
	}
	bool stringImpl(const std::string &) override
	{
		return reportError("Invalid value type 'null', expected 'object' or 'array'");
	}
	bool startObjectImpl() override
	{
		m_container->insert({m_currentKey, std::make_shared<Schema>()});
		delegateToObject(new SchemaParserHandler(m_container->at(m_currentKey).get<SchemaPtr>()));
		return true;
	}
	bool keyImpl(const std::string &str) override
	{
		m_currentKey = str;
		return true;
	}
	bool endObjectImpl(const std::size_t) override
	{
		ASSERT(false);
		return true;
	}
	bool startArrayImpl() override
	{
		m_container->insert({m_currentKey, ArrayContainerT<StringT>()});
		delegateToArray(new StringArrayParser(&m_container->at(m_currentKey).get<ArrayContainerT<StringT>>()));
		return true;
	}
	bool endArrayImpl(const std::size_t) override
	{
		ASSERT(false);
		return true;
	}
};

SchemaParserHandler::SchemaParserHandler(SchemaPtr schema)
	: m_schema(schema)
{
}

bool SchemaParserHandler::nullImpl()
{
	return reportError("Invalid value: null not expected for '%s'", m_currentKey.c_str());
}
bool SchemaParserHandler::booleanImpl(const bool val)
{
	if (m_currentKey == "default") {
		m_schema->default_ = val;
	} else if (m_currentKey == "exclusiveMaximum") {
		m_schema->exclusiveMaximum = val;
	} else if (m_currentKey == "exclusiveMinimum") {
		m_schema->exclusiveMinimum = val;
	} else if (m_currentKey == "uniqueItems") {
		m_schema->uniqueItems = val;
	} else if (m_currentKey == "additionalItems") {
		m_schema->additionalItems = val;
	} else if (m_currentKey == "additionalProperties") {
		m_schema->additionalProperties = val;
	} else {
		return reportError("Invalid value: boolean not expected for '%s'", m_currentKey.c_str());
	}
	return true;
}
bool SchemaParserHandler::integerNumberImpl(const int64_t val)
{
	if (m_currentKey == "default") {
		m_schema->default_ = val;
	} else if (m_currentKey == "multipleOf") {
		if (val <= 0) {
			return reportError("multipleOf must be bigger than 0");
		} else {
			m_schema->multipleOf = val;
		}
	} else if (m_currentKey == "maximum") {
		m_schema->maximum = val;
	} else if (m_currentKey == "minimum") {
		m_schema->minimum = val;
	} else if (m_currentKey == "minLength") {
		if (val < 0) {
			return reportError("minLength must be bigger than or equal 0");
		} else {
			m_schema->minLength = val;
		}
	} else if (m_currentKey == "maxLength") {
		if (val < 0) {
			return reportError("maxLength must be bigger than or equal 0");
		} else {
			m_schema->maxLength = val;
		}
	} else if (m_currentKey == "minItems") {
		if (val < 0) {
			return reportError("minItems must be bigger than or equal 0");
		} else {
			m_schema->minItems = val;
		}
	} else if (m_currentKey == "maxItems") {
		if (val < 0) {
			return reportError("maxItems must be bigger than or equal 0");
		} else {
			m_schema->maxItems = val;
		}
	} else if (m_currentKey == "minProperties") {
		if (val < 0) {
			return reportError("minProperties must be bigger than or equal 0");
		} else {
			m_schema->minProperties = val;
		}
	} else if (m_currentKey == "maxProperties") {
		if (val < 0) {
			return reportError("maxProperties must be bigger than or equal 0");
		} else {
			m_schema->maxProperties = val;
		}
	} else {
		return reportError("Invalid value: integer not expected for '%s'", m_currentKey.c_str());
	}
	return true;
}
bool SchemaParserHandler::doubleNumberImpl(const double val)
{
	if (m_currentKey == "default") {
		m_schema->default_ = val;
	} else {
		return reportError("Invalid value: double not expected for '%s'", m_currentKey.c_str());
	}
	return true;
}
bool SchemaParserHandler::stringImpl(const std::string &str)
{
	if (m_currentKey == "default") {
		m_schema->default_ = str;
	} else if (m_currentKey == "$ref") {
		if (!Json::isURI(str)) {
			return reportError("Invalid value: value for '%s' is not a URI", m_currentKey.c_str());
		} else {
			m_schema->ref = str;
		}
	} else if (m_currentKey == "$schema") {
		if (str != "http://json-schema.org/draft-04/schema#") {
			return reportError("Invalid schema: v4 required");
		} else {
			m_schema->schema = str;
		}
	} else if (m_currentKey == "id") {
		m_schema->id = str;
	} else if (m_currentKey == "title") {
		m_schema->title = str;
	} else if (m_currentKey == "description") {
		m_schema->description = str;
	} else if (m_currentKey == "pattern") {
		if (!Json::isRegex(str)) {
			return reportError("Invalid value: valid for '%s' is not a regex", m_currentKey.c_str());
		} else {
			m_schema->pattern = str;
		}
	} else if (m_currentKey == "type") {
		Type type = TypeArrayParser::typeFromString(str);
		if (type == Type::Invalid) {
			return reportError("Invalid string value '%s', expected one of 'array', 'boolean', 'number', 'null', 'object', 'string'", str.c_str());
		} else {
			m_schema->type = type;
		}
	} else if (m_currentKey == "format") {
		m_schema->format = str;
	}else {
		return reportError("Invalid value: string not expected for '%s'", m_currentKey.c_str());
	}
	return true;
}
bool SchemaParserHandler::startObjectImpl()
{
	if (m_currentKey.empty()) {
		// no-op, this is the start of the schema object
	} else if (m_currentKey == "default") {
		delegateToObject(Json::Value::parserSink(&m_schema->default_));
	} else if (m_currentKey == "additionalItems") {
		m_schema->additionalItems = std::make_shared<Schema>();
		delegateToObject(new SchemaParserHandler(m_schema->additionalItems.get<SchemaPtr>()));
	} else if (m_currentKey == "items") {
		m_schema->items = std::make_shared<Schema>();
		delegateToObject(new SchemaParserHandler(m_schema->items.get<SchemaPtr>()));
	} else if (m_currentKey == "additionalProperties") {
		m_schema->additionalProperties = std::make_shared<Schema>();
		delegateToObject(new SchemaParserHandler(m_schema->items.get<SchemaPtr>()));
	} else if (m_currentKey == "not") {
		m_schema->not_ = std::make_shared<Schema>();
		delegateToObject(new SchemaParserHandler(m_schema->not_));
	} else if (m_currentKey == "definitions") {
		delegateToObject(new SchemaMapParser(&m_schema->definitions));
	} else if (m_currentKey == "properties") {
		delegateToObject(new SchemaMapParser(&m_schema->properties));
	} else if (m_currentKey == "patternProperties") {
		delegateToObject(new SchemaMapParser(&m_schema->patternProperties));
	} else if (m_currentKey == "dependencies") {
		delegateToObject(new DependenciesParser(&m_schema->dependencies));
	} else {
		return reportError("Invalid value: object not expected for '%s'", m_currentKey.c_str());
	}
	return true;
}
bool SchemaParserHandler::keyImpl(const std::string &str)
{
	m_currentKey = str;
	return true;
}
bool SchemaParserHandler::endObjectImpl(const std::size_t)
{
	return true;
}
bool SchemaParserHandler::startArrayImpl()
{
	if (m_currentKey == "items") {
		delegateToArray(new SchemaArrayParser(&m_schema->items.get<ArrayContainerT<SchemaPtr>>()));
	} else if (m_currentKey == "required") {
		delegateToArray(new StringArrayParser(&m_schema->required));
	} else if (m_currentKey == "enum") {
		delegateToArray(new StringArrayParser(&m_schema->enum_));
	} else if (m_currentKey == "type") {
		m_schema->type = ArrayContainerT<Type>();
		delegateToArray(new TypeArrayParser(&m_schema->type.get<ArrayContainerT<Type>>()));
	} else if (m_currentKey == "allOf") {
		delegateToArray(new SchemaArrayParser(&m_schema->allOf));
	} else if (m_currentKey == "anyOf") {
		delegateToArray(new SchemaArrayParser(&m_schema->anyOf));
	} else if (m_currentKey == "oneOf") {
		delegateToArray(new SchemaArrayParser(&m_schema->oneOf));
	}else {
		return reportError("Invalid value: array not expected for '%s'", m_currentKey.c_str());
	}
	return true;
}
bool SchemaParserHandler::endArrayImpl(const std::size_t)
{
	return true;
}
}
}
}
