#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "util/Variant.h"

namespace Argonauts {
namespace Tool {

struct Annotations
{
	using Value = Util::Variant<std::string, int64_t>;

	std::unordered_map<std::string, Value> values;
};

struct Type
{
	using Ptr = std::shared_ptr<Type>;

	bool isInteger() const
	{
		return name == "Int8" || name == "Int16" || name == "Int32" || name == "Int64" ||
				name == "UInt8" || name == "UInt16" || name == "UInt32" || name == "UInt64";
	}
	bool isSimple() const
	{
		return isInteger() || name == "String" || name == "Double" || name == "Bool";
	}
	bool isBuiltin() const
	{
		return isSimple() || name == "List" || name == "Variant" || name == "Map";
	}
	bool isObjectish() const
	{
		return !isSimple() && name != "List" && name != "Variant";
	}
	bool isTemplateable() const
	{
		return name == "List" || name == "Variant" || name == "Map";
	}
	bool isTemplated() const
	{
		return !templateArguments.empty();
	}

	std::vector<std::string> namesRecursive() const;
	std::vector<Type::Ptr> allOfTypeRecursive(const Type::Ptr &self, const std::string &type) const;
	std::vector<Type::Ptr> allRecursive(const Type::Ptr &self) const;

	bool compare(const Ptr &other) const;

	std::string name;
	std::vector<Type::Ptr> templateArguments;

	explicit Type(const std::string &name_, std::vector<Type::Ptr> &&args) : name(name_), templateArguments(std::forward<std::vector<Type::Ptr>>(args)) {}
	template <typename... Args>
	static Type::Ptr create(const std::string &name, Args &&... args) { return std::make_shared<Type>(name, std::vector<Type::Ptr>({std::forward<std::vector<Type::Ptr>>(args)...})); }
};

struct Attribute
{
	Type::Ptr type;

	int index;
	std::string name;

	Annotations annotations;

	explicit Attribute(Type::Ptr &type_, const int index_, const std::string &name_, const Annotations &annotations_)
		: type(std::forward<Type::Ptr>(type_)), index(index_), name(name_), annotations(annotations_) {}
};
struct Struct
{
	std::string name;
	std::vector<Attribute> members;

	Annotations annotations;

	std::vector<Type::Ptr> allTypes() const;
};
struct EnumEntry
{
	std::string name;
	int64_t value;

	Annotations annotations;

	explicit EnumEntry(const std::string &name_, const int64_t value_, const Annotations &annotations_)
		: name(name_), value(value_), annotations(annotations_) {}
};
struct Enum
{
	std::string name;
	std::string type;
	std::vector<EnumEntry> entries;

	Annotations annotations;
};

struct File
{
	std::vector<Struct> structs;
	std::vector<Enum> enums;
};
}
}
