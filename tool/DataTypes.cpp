#include "DataTypes.h"

#include "util/StringUtil.h"

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

}
}
