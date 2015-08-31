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

#include "TypeProviders.h"

#include <iostream>
#include <algorithm>

#include "DataTypes.h"
#include "util/StringUtil.h"

namespace Argonauts {
namespace Tool {
TypeProvider::~TypeProvider() {}

std::string TypeProvider::fullType(const Attribute &attribute) const
{
	return fullType(attribute.type);
}

std::string TypeProvider::fullType(const std::shared_ptr<Type> &t) const
{
	if (t->isTemplated()) {
		std::vector<std::string> args;
		std::transform(t->templateArguments.begin(), t->templateArguments.end(), std::back_inserter(args), [this](const Type::Ptr &ptr) { return fullType(ptr); });
		return type(t->name) + '<' + Util::String::joinStrings(args, ", ") + '>';
	} else {
		return type(t->name);
	}
}

bool TypeProvider::isIntegerType(const std::string &type) const
{
	return type == "Int8" || type == "Int16" || type =="Int32" || type == "Int64" ||
			type == "UInt8" || type == "UInt16" || type == "UInt32" || type == "UInt64";
}

bool TypeProvider::isObjectType(const std::string &type) const
{
	return !isIntegerType(type) && type != "Double" && type != "String" && type != "List";
}

std::string TypeProvider::getFromMapHelper(const std::unordered_map<std::string, std::string> &map, const std::string &key, const std::string &default_) const
{
	if (map.find(key) == map.end())
	{
		return default_;
	}
	else
	{
		return map.at(key);
	}
}

std::string QtTypeProvider::type(const std::string &type) const
{
	std::unordered_map<std::string, std::string> mapping;
	mapping["Int8"] = "qint8";
	mapping["Int16"] = "qint16";
	mapping["Int32"] = "qint32";
	mapping["Int64"] = "qint64";
	mapping["UInt8"] = "quint8";
	mapping["UInt16"] = "quint16";
	mapping["UInt32"] = "quint32";
	mapping["UInt64"] = "quint64";
	mapping["String"] = "QString";
	mapping["List"] = "QVector";
	mapping["Map"] = "QHash";
	mapping["Bool"] = "bool";
	mapping["Variant"] = "Argonauts::Util::Variant";
	return getFromMapHelper(mapping, type, type);
}

std::string QtTypeProvider::headerForType(const std::string &type) const
{
	std::unordered_map<std::string, std::string> mapping;
	mapping["Int8"] = "QtGlobal";
	mapping["Int16"] = "QtGlobal";
	mapping["Int32"] = "QtGlobal";
	mapping["Int64"] = "QtGlobal";
	mapping["UInt8"] = "QtGlobal";
	mapping["UInt16"] = "QtGlobal";
	mapping["UInt32"] = "QtGlobal";
	mapping["UInt64"] = "QtGlobal";
	mapping["String"] = "QString";
	mapping["List"] = "QVector";
	mapping["Map"] = "QHash";
	mapping["Bool"] = std::string();
	mapping["Variant"] = "util/Variant.h";
	return getFromMapHelper(mapping, type, type + ".arg.h");
}

std::string STLTypeProvider::type(const std::string &type) const
{
	std::unordered_map<std::string, std::string> mapping;
	mapping["Int8"] = "std::int8_t";
	mapping["Int16"] = "std::int16_t";
	mapping["Int32"] = "std::int32_t";
	mapping["Int64"] = "std::int64_t";
	mapping["UInt8"] = "std::uint8_t";
	mapping["UInt16"] = "std::uint16_t";
	mapping["UInt32"] = "std::uint32_t";
	mapping["UInt64"] = "std::uint64_t";
	mapping["String"] = "std::string";
	mapping["List"] = "std::vector";
	mapping["Map"] = "std::unordered_map";
	mapping["Bool"] = "bool";
	mapping["Variant"] = "Argonauts::Util::Variant";
	return getFromMapHelper(mapping, type, type);
}

std::string STLTypeProvider::headerForType(const std::string &type) const
{
	std::unordered_map<std::string, std::string> mapping;
	mapping["Int8"] = "cstdint";
	mapping["Int16"] = "cstdint";
	mapping["Int32"] = "cstdint";
	mapping["Int64"] = "cstdint";
	mapping["UInt8"] = "cstdint";
	mapping["UInt16"] = "cstdint";
	mapping["UInt32"] = "cstdint";
	mapping["UInt64"] = "cstdint";
	mapping["String"] = "string";
	mapping["List"] = "vector";
	mapping["Map"] = "unordered_map";
	mapping["Bool"] = std::string();
	mapping["Variant"] = "util/Variant.h";
	return getFromMapHelper(mapping, type, type + ".arg.h");
}
}
}
