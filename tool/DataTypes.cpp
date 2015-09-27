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

#include "DataTypes.h"

#include <algorithm>

#include "util/StringUtil.h"
#include "util/Error.h"

#include "common/Lexer.h"
#include "Parser.h"
#include "Resolver.h"

namespace Argonauts {
namespace Tool {

std::vector<std::string> Type::integers()
{
	return {
		"Int8",
		"Int16",
		"Int32",
		"Int64",
		"UInt8",
		"UInt16",
		"UInt32",
		"UInt64"
	};
}

std::vector<std::string> Type::namesRecursive() const
{
	std::vector<std::string> out = {name};
	for (const Type::Ptr &ptr : templateArguments) {
		const std::vector<std::string> n = ptr->namesRecursive();
		std::copy(n.begin(), n.end(), std::back_inserter(out));
	}
	return out;
}
std::vector<Type::Ptr> Type::allOfTypeRecursive(const Type::Ptr &self, const std::string &type) const
{
	std::vector<Type::Ptr> out;
	if (name == type) {
		out.push_back(self);
	}
	for (const Type::Ptr &child : templateArguments) {
		const std::vector<Type::Ptr> res = child->allOfTypeRecursive(child, type);
		std::copy(res.begin(), res.end(), std::back_inserter(out));
	}
	return out;
}
std::vector<Type::Ptr> Type::allRecursive(const Type::Ptr &self) const
{
	std::vector<Type::Ptr> out = {self};
	for (const Type::Ptr &child : templateArguments) {
		const std::vector<Type::Ptr> fromChild = child->allRecursive(child);
		std::copy(fromChild.begin(), fromChild.end(), std::back_inserter(out));
	}
	return out;
}
bool Type::compare(const Type::Ptr &other) const
{
	if (name != other->name || templateArguments.size() != other->templateArguments.size()) {
		return false;
	}
	for (std::size_t i = 0; i < templateArguments.size(); ++i) {
		if (!templateArguments.at(i)->compare(other->templateArguments.at(i))) {
			return false;
		}
	}
	return true;
}

std::string Type::toString() const
{
	std::string out = name;
	if (!templateArguments.empty()) {
		std::vector<std::string> children;
		for (const auto &child : templateArguments) {
			children.push_back(child->toString());
		}
		out += '<' + Util::String::joinStrings(children, ", ") + '>';
	}
	return out;
}

std::vector<Type::Ptr> Struct::allTypes() const
{
	std::vector<Type::Ptr> types;
	for (const Attribute &attribute : members) {
		const std::vector<Type::Ptr> t = attribute.type->allRecursive(attribute.type);
		for (const Type::Ptr &type : t) {
			bool found = false;
			for (const Type::Ptr &existing : types) {
				if (type->compare(existing)) {
					found = true;
					break;
				}
			}

			if (!found) {
				types.push_back(type);
			}
		}
	}
	return types;
}

std::string Annotations::getString(const std::string &name, const std::string &def) const
{
	if (!contains(name)) {
		return def;
	}
	const Value value = values.find(name)->second;
	if (value.is<PositionedInt64>()) {
		return std::to_string(value.get<PositionedInt64>());
	} else {
		return value.get<PositionedString>();
	}
}
std::vector<std::string> Annotations::getStrings(const std::string &name) const
{
	std::vector<std::string> out;
	for (const auto &pair : values) {
		if (pair.first == name) {
			out.push_back(pair.second.get<PositionedString>());
		}
	}
	return out;
}
int64_t Annotations::getInt(const std::string &name) const
{
	return contains(name) ? values.find(name)->second.get<PositionedInt64>().value : -1;
}
bool Annotations::isString(const std::string &name) const
{
	return contains(name) && values.find(name)->second.is<PositionedString>();
}

File lexAndParse(const std::string &data, const std::string &filename, const int flags)
{
	try {
		Resolver resolver(Parser(Common::Lexer().consume(data, filename)).process());
		if (flags & ResolveAliases) {
			resolver.resolveAliases();
		}
		if (flags & ResolveIncludes) {
			resolver.resolveIncludes();
		}
		return resolver.result();
	} catch (Parser::ParserException &e) {
		throw Util::Error(e.what(), e.offset, Util::Error::Source(data, filename));
	} catch (Resolver::ResolverError &e) {
		throw Util::Error(e.what(), e.offset, Util::Error::Source(data, filename));
	}
}

std::vector<std::string> File::definedTypes() const
{
	std::vector<std::string> out;
	for (const Struct &s : structs) {
		out.push_back(s.name);
	}
	for (const Enum &e : enums) {
		out.push_back(e.name);
	}
	for (const Using &u : usings) {
		out.push_back(u.name);
	}
	return out;
}

std::vector<Type::Ptr> File::allTypes() const
{
	std::vector<Type::Ptr> types;
	for (const Struct &s : structs) {
		const std::vector<Type::Ptr> add = s.allTypes();
		std::copy_if(add.begin(), add.end(), std::back_inserter(types), [&types](const Type::Ptr &type)
		{
			return std::find_if(types.begin(), types.end(), [type](const Type::Ptr &other) { return other->compare(type); }) == types.end();
		});
	}
	return types;
}

}
}
