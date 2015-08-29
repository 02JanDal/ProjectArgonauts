#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace Argonauts {
namespace Util {
namespace Json {
class Pointer;
using SchemaPtr = std::shared_ptr<class Schema>;

// TODO: allow giving a callback that fetches an external target (runtime caching done by this class)
class SchemaResolver
{
	std::unordered_map<std::string, SchemaPtr> m_schemas;
	const int m_flags;
public:
	enum Flags
	{
		NoFlags = 0x00,
		ExternalReferenceBasenameOnly = 0x01
	};

	explicit SchemaResolver(const std::unordered_map<std::string, SchemaPtr> &schemas = {}, const int flags = NoFlags);
	explicit SchemaResolver(const int flags);

	// 1. add schemas
	void addSchema(const std::string &name, const SchemaPtr &schema);

	// 2. run resolve
	void resolve();

	// 3. retrieve results
	std::vector<SchemaPtr> result() const;
	std::vector<std::string> usedSchemas() const;
	std::unordered_map<std::string, SchemaPtr> rawResult() const { return m_schemas; }

	class ResolvationError : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

private:
	void resolve(SchemaPtr root, SchemaPtr ptr);
	// we never have the full JSON Schema in memory as a Json::Value, so we need to manually dereference the pointer
	SchemaPtr resolvePointer(const SchemaPtr &base, const Pointer &pointer);
};
}
}
}
