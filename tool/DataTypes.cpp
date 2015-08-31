#include "DataTypes.h"

#include "util/StringUtil.h"
#include "util/Error.h"

#include "Lexer.h"
#include "Parser.h"
#include "Resolver.h"

namespace Argonauts {
namespace Tool {

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
	if (value.value.is<int64_t>()) {
		return std::to_string(value.value.get<int64_t>());
	} else {
		return value.value.get<std::string>();
	}
}
std::vector<std::string> Annotations::getStrings(const std::string &name) const
{
	std::vector<std::string> out;
	for (const auto &pair : values) {
		if (pair.first == name) {
			out.push_back(pair.second.value.get<std::string>());
		}
	}
	return out;
}
int64_t Annotations::getInt(const std::string &name) const
{
	return contains(name) ? values.find(name)->second.value.get<int64_t>() : -1;
}

File lexAndParse(const std::string &data, const std::string &filename, const int flags)
{
	try {
		Resolver resolver(Parser(Lexer().consume(data, filename)).process());
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

}
}
