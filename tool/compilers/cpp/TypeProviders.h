#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace Argonauts {
namespace Tool {
struct Attribute;
struct Type;

class TypeProvider
{
public:
	virtual ~TypeProvider();
	virtual std::string type(const std::string &type) const = 0;
	virtual std::string headerForType(const std::string &type) const = 0;

	virtual std::string listAppendFunction() const = 0;
	virtual std::string listSizeFunction() const = 0;
	virtual std::string listAtFunction() const = 0;
	virtual std::string listIndexType() const = 0;

	std::string fullType(const Attribute &attribute) const;
	std::string fullType(const std::shared_ptr<Type> &t) const;
	bool isIntegerType(const std::string &type) const;
	bool isObjectType(const std::string &type) const;

protected:
	std::string getFromMapHelper(const std::unordered_map<std::string, std::string> &map, const std::string &key, const std::string &default_) const;
};
class QtTypeProvider : public TypeProvider
{
public:
	std::string type(const std::string &type) const override;
	std::string headerForType(const std::string &type) const override;

	std::string listAppendFunction() const override { return "append"; }
	std::string listSizeFunction() const override { return "size"; }
	std::string listAtFunction() const override { return "at"; }
	std::string listIndexType() const override { return "int"; }
};
class STLTypeProvider : public TypeProvider
{
	std::string type(const std::string &type) const override;
	std::string headerForType(const std::string &type) const override;

	std::string listAppendFunction() const override { return "push_back"; }
	std::string listSizeFunction() const override { return "size"; }
	std::string listAtFunction() const override { return "at"; }
	std::string listIndexType() const override { return "std::size_t"; }
};
}
}
