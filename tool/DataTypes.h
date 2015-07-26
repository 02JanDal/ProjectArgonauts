#pragma once

#include <vector>
#include <string>

namespace Argonauts
{
struct AnnotationArgument
{
	std::string key;
	std::string valueString;
	int64_t valueInteger;
	enum { String, Integer, Identifier } valueType;
};
struct Annotation
{
	std::string name;
	std::vector<AnnotationArgument> arguments;
};

struct Attribute
{
	std::string type;
	std::string templateType;

	int index;
	std::string name;

	std::vector<Annotation> annotations;

	explicit Attribute(const std::string &type, const std::string &templateType, const int index, const std::string &name, const std::vector<Annotation> &annotations)
		: type(type), templateType(templateType), index(index), name(name), annotations(annotations) {}
};
struct Struct
{
	std::string name;
	std::vector<Attribute> members;

	std::vector<Annotation> annotations;
};
struct EnumEntry
{
	std::string name;
	int64_t value;

	std::vector<Annotation> annotations;

	explicit EnumEntry(const std::string &name, const int64_t value, const std::vector<Annotation> &annotations)
		: name(name), value(value), annotations(annotations) {}
};
struct Enum
{
	std::string name;
	std::string type;
	std::vector<EnumEntry> entries;

	std::vector<Annotation> annotations;
};

struct File
{
	std::vector<Struct> structs;
	std::vector<Enum> enums;
};
}
