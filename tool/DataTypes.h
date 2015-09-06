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

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "util/Variant.h"

namespace Argonauts {
namespace Tool {

// Wraps a value, like a string or an integer, with information about where in the input stream it occured
template <typename T>
struct PositionedValue
{
	PositionedValue(const T &value_ = T(), const int offset_ = -1, const int length_ = -1) : value(value_), offset(offset_), length(length_) {}

	T value;
	int offset, length;

	operator T() const { return value; }

	bool operator==(const T &other) const { return value == other; }
	bool operator!=(const T &other) const { return value != other; }
	bool operator==(const PositionedValue<T> &other) const { return value == other.value; }
	bool operator!=(const PositionedValue<T> &other) const { return value == other.value; }
};
using PositionedString = PositionedValue<std::string>;
using PositionedInt64 = PositionedValue<int64_t>;
}
}
template <typename T>
T operator+(const T &a, const Argonauts::Tool::PositionedValue<T> &b)
{
	return a + b.value;
}

namespace std {
template <>
struct hash<Argonauts::Tool::PositionedString>
{
	std::size_t operator()(const Argonauts::Tool::PositionedString &string) const
	{
		return std::hash<std::string>()(string.value);
	}
};
inline std::string to_string(const Argonauts::Tool::PositionedString &str) { return str.value; }
}

namespace Argonauts {
namespace Tool {

struct Annotations
{
	using Value = Util::Variant<PositionedString, PositionedInt64>;

	std::unordered_multimap<PositionedString, Value> values;

	bool contains(const std::string &name) const { return values.find(name) != values.end(); }
	std::string getString(const std::string &name, const std::string &def = std::string()) const;
	std::vector<std::string> getStrings(const std::string &name) const;
	int64_t getInt(const std::string &name) const;

	// convenience: doc.*
	bool hasDocumentation() const { return contains("doc") || contains("doc.brief"); }
	std::string docBrief() const { return getString(contains("doc.brief") ? "doc.brief" : "doc"); }
	std::string docExtended() const { return getString("doc.extended"); }
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

	std::string toString() const;

	PositionedString name;
	std::vector<Type::Ptr> templateArguments;

	explicit Type(const PositionedString &name_, std::vector<Type::Ptr> &&args) : name(name_), templateArguments(std::forward<std::vector<Type::Ptr>>(args)) {}
	template <typename... Args>
	static Type::Ptr create(const PositionedString &name, Args &&... args) { return std::make_shared<Type>(name, std::vector<Type::Ptr>({std::forward<std::vector<Type::Ptr>>(args)...})); }
};

struct Attribute
{
	Type::Ptr type;

	PositionedInt64 index;
	PositionedString name;

	Annotations annotations;

	explicit Attribute(const Type::Ptr &type_, const PositionedInt64 index_, const PositionedString &name_, const Annotations &annotations_)
		: type(type_), index(index_), name(name_), annotations(annotations_) {}
};
struct Struct
{
	PositionedString includes;
	PositionedString name;
	std::vector<Attribute> members;

	Annotations annotations;

	std::vector<Type::Ptr> allTypes() const;
};
struct EnumEntry
{
	PositionedString name;
	PositionedInt64 value;

	Annotations annotations;

	explicit EnumEntry(const PositionedString &name_, const PositionedInt64 value_, const Annotations &annotations_)
		: name(name_), value(value_), annotations(annotations_) {}
};
struct Enum
{
	PositionedString name;
	PositionedString type;
	std::vector<EnumEntry> entries;

	Annotations annotations;
};

struct Using
{
	PositionedString name;
	Type::Ptr type;
	Annotations annotations;
};

struct File
{
	std::vector<Struct> structs;
	std::vector<Enum> enums;
	std::vector<Using> usings;

	std::vector<std::string> definedTypes() const;
};

enum LexAndParseFlags
{
	NoFlags = 0x0,
	ResolveAliases = 0x1,
	ResolveIncludes = 0x2
};
File lexAndParse(const std::string &data, const std::string &filename, const int flags = 0);

}
}
