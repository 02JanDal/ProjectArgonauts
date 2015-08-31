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

#include "Resolver.h"

#include <algorithm>

namespace Argonauts {
namespace Tool {

Resolver::Resolver(const File &file)
	: m_file(file)
{
}

static Annotations mergeAnnotations(const Annotations &a, const Annotations &b)
{
	Annotations out = a;
	std::copy_if(b.values.begin(), b.values.end(), std::inserter(out.values, out.values.begin()), [](const auto &pair)
	{
		return pair.first != "hidden";
	});
	return out;
}

static void resolveAliasesHelper(const std::unordered_map<PositionedString, Using> &aliases, const Type::Ptr &type)
{
	if (aliases.find(type->name) != aliases.end()) {
		const Type::Ptr alias = aliases.find(type->name)->second.type;
		type->name = alias->name;
		type->templateArguments = alias->templateArguments;
		resolveAliasesHelper(aliases, type);
	}
}
void Resolver::resolveAliases()
{
	std::unordered_map<PositionedString, Using> aliases;
	for (const Using &alias : m_file.usings) {
		aliases.insert({alias.name, alias});
	}

	for (const Struct &structure : m_file.structs) {
		for (const Attribute &attribute : structure.members) {
			resolveAliasesHelper(aliases, attribute.type);
		}
	}
}

static std::vector<Attribute> mergeAttributes(const std::vector<Attribute> &a, const std::vector<Attribute> &b)
{
	std::vector<Attribute> out = a;
	for (const Attribute &attribute : b) {
		auto it = std::find_if(out.begin(), out.end(), [attribute](const Attribute &attr) { return attr.name == attribute.name; });
		if (it != out.end()) {
			out.erase(it);
		}
		out.push_back(attribute);
	}
	return out;
}
static void resolveIncludesHelper(std::unordered_map<PositionedString, Struct> &structs, Struct &structure)
{
	if (!structure.includes.value.empty()) {
		auto it = structs.find(structure.includes.value);
		if (it == structs.end()) {
			throw Resolver::ResolverError(std::string("Unable to resolve struct include: Cannot find parent of '") + structure.name + "'", structure.includes.position);
		} else {
			structure.members = mergeAttributes(it->second.members, structure.members);
			structure.includes = it->second.includes;
			structure.annotations = mergeAnnotations(structure.annotations, it->second.annotations);
			structs.insert({structure.name, structure});
			resolveIncludesHelper(structs, structure);
		}
	}
}
void Resolver::resolveIncludes()
{
	std::unordered_map<PositionedString, Struct> structs;
	for (const Struct &structure : m_file.structs) {
		structs.insert({structure.name, structure});
	}

	for (Struct &structure : m_file.structs) {
		resolveIncludesHelper(structs, structure);
	}
}

}
}
