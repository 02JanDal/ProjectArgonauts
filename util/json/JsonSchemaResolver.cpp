#include "JsonSchemaResolver.h"

#include <algorithm>

#include "JsonSchema.h"
#include "JsonReference.h"
#include "JsonPointer.h"

namespace Argonauts {
namespace Util {
namespace Json {
SchemaResolver::SchemaResolver(const std::unordered_map<std::string, SchemaPtr> &schemas, const int flags)
	: m_schemas(schemas), m_flags(flags)
{
}
SchemaResolver::SchemaResolver(const int flags)
	: m_flags(flags)
{
}

void SchemaResolver::addSchema(const std::string &name, const SchemaPtr &schema)
{
	m_schemas.insert({name, schema});
}

void SchemaResolver::resolve()
{
	std::unordered_map<std::string, SchemaPtr> cleaned;
	for (const auto &pair : m_schemas) {
		SchemaPtr schema = pair.second;
		resolve(schema, schema);
	}
}

std::vector<SchemaPtr> SchemaResolver::result() const
{
	std::vector<SchemaPtr> out;
	for (const auto &pair : m_schemas) {
		out.push_back(pair.second);
	}
	return out;
}
std::vector<std::string> SchemaResolver::usedSchemas() const
{
	std::vector<std::string> out;
	for (const auto &pair : m_schemas) {
		out.push_back(pair.first);
	}
	return out;
}

void SchemaResolver::resolve(SchemaPtr root, SchemaPtr ptr)
{
	if (!ptr) {
		return;
	}

	const std::string initialRef = ptr->ref;
	while (!ptr->ref.empty()) {
		const Reference ref(ptr->ref);

		if (!ref.isValid()) {
			throw ResolvationError("Unable to resolve a JSON Schema: given $ref is not a valid JSON Reference");
		}

		SchemaPtr targetRootSchema;
		if (ref.uri().empty()) {
			targetRootSchema = root;
		} else {
			auto it = std::find_if(m_schemas.begin(), m_schemas.end(), [this, ref](const auto &pair)
			{
				if (m_flags & ExternalReferenceBasenameOnly) {
					std::string target = ref.uri();
					target = target.substr(target.rfind('/'));
					target = target.substr(0, target.find('.'));
					std::string candidate = pair.first;
					candidate = candidate.substr(candidate.rfind('/'));
					candidate = candidate.substr(0, candidate.find('.'));
					return target == candidate;
				} else {
					return pair.first == ref.uri();
				}
			});
			if (it == m_schemas.end()) {
				throw ResolvationError("Unable to resolve a JSON Schema: external target not found");
			} else {
				targetRootSchema = (*it).second;
			}
		}

		try {
			SchemaPtr targetSchema = resolvePointer(targetRootSchema, ref.pointer());
			*ptr = *targetSchema;
		} catch (ResolvationError &) {
			std::throw_with_nested(ResolvationError("Unable to resolve a JSON Schema: internal target not found"));
		}

		if (!ptr->ref.empty() && ptr->ref == initialRef) {
			throw ResolvationError("Unable to resolve a JSON Schema: cyclic reference detected");
		}
	}

	// resolve subschemas
	if (ptr->additionalItems.is<SchemaPtr>()) {
		resolve(root, ptr->additionalItems.get<SchemaPtr>());
	}
	if (ptr->items.is<SchemaPtr>()) {
		resolve(root, ptr->items.get<SchemaPtr>());
	} else if (ptr->items.is<ArrayContainerT<SchemaPtr>>()) {
		for (const SchemaPtr &schema : ptr->items.get<ArrayContainerT<SchemaPtr>>()) {
			resolve(root, schema);
		}
	}
	if (ptr->additionalProperties.is<SchemaPtr>()) {
		resolve(root, ptr->additionalProperties.get<SchemaPtr>());
	}
	for (const auto &pair : ptr->definitions) {
		resolve(root, pair.second);
	}
	for (const auto &pair : ptr->properties) {
		resolve(root, pair.second);
	}
	for (const auto &pair : ptr->patternProperties) {
		resolve(root, pair.second);
	}
	for (const auto &pair : ptr->dependencies) {
		if (pair.second.is<SchemaPtr>()) {
			resolve(root, pair.second.get<SchemaPtr>());
		}
	}
	for (const SchemaPtr &schema : ptr->allOf) {
		resolve(root, schema);
	}
	for (const SchemaPtr &schema : ptr->anyOf) {
		resolve(root, schema);
	}
	for (const SchemaPtr &schema : ptr->oneOf) {
		resolve(root, schema);
	}
	resolve(root, ptr->not_);
}

SchemaPtr SchemaResolver::resolvePointer(const SchemaPtr &base, const Pointer &pointer)
{
	SchemaPtr result = base;
	std::vector<std::string> tokens = pointer.referenceTokens();
	auto it = tokens.begin();

	auto resolveIntoObject = [&it, tokens](const ObjectContainerT<SchemaPtr> &object) -> SchemaPtr
	{
		if (it == tokens.end()) {
			throw ResolvationError("Attempted to reference into object-of-schemas without a key");
		} else {
			try {
				return object.at(*it++);
			} catch (std::out_of_range &) {
				std::throw_with_nested(ResolvationError("Attempted to reference non-existing object member"));
			}
		}
	};
	auto resolveIntoArray = [&it, tokens](const ArrayContainerT<SchemaPtr> &array) -> SchemaPtr
	{
		if (it == tokens.end()) {
			throw ResolvationError("Attempted to reference into array-of-schemas without an index");
		} else {
			try {
				return array.at(std::stoi(*it++));
			} catch (std::invalid_argument &) {
				std::throw_with_nested(ResolvationError("Attempted to reference into array-of-schemas with a non-integer"));
			} catch (std::out_of_range &) {
				std::throw_with_nested(ResolvationError("Attempted to reference into array-of-schemas at non-existing index"));
			}
		}
	};

	while (it != tokens.end()) {
		ASSERT(result.get());

		const std::string &next = *it++;
		if (next == "additionalItems") {
			if (result->additionalItems.is<SchemaPtr>()) {
				result = result->additionalItems.get<SchemaPtr>();
			} else {
				throw ResolvationError("Attempted to reference 'additionalItems' which is not a schema");
			}
		} else if (next == "items") {
			if (result->items.is<SchemaPtr>()) {
				result = result->items.get<SchemaPtr>();
			} else if (result->items.is<ArrayContainerT<SchemaPtr>>()) {
				result = resolveIntoArray(result->items.get<ArrayContainerT<SchemaPtr>>());
			} else {
				throw ResolvationError("Attempt to reference 'items' which is not a schema");
			}
		} else if (next == "additionalProperties") {
			if (result->additionalProperties.is<SchemaPtr>()) {
				result = result->additionalProperties.get<SchemaPtr>();
			} else {
				throw ResolvationError("Attempted to reference 'additionalProperties' which is not a schema");
			}
		} else if (next == "definitions") {
			result = resolveIntoObject(result->definitions);
		} else if (next == "properties") {
			result = resolveIntoObject(result->properties);
		} else if (next == "patternProperties") {
			result = resolveIntoObject(result->patternProperties);
		} else if (next == "dependencies") {
			if (it == tokens.end()) {
				throw ResolvationError("Attempted to reference into object-of-schemas without a key");
			} else {
				try {
					auto var = result->dependencies.at(*it++);
					if (var.is<SchemaPtr>()) {
						result = var.get<SchemaPtr>();
					} else {
						throw ResolvationError("Attempted to reference item of 'dependencies' which is not a schema");
					}
				} catch (std::out_of_range &) {
					std::throw_with_nested(ResolvationError("Attempted to reference non-existing object member"));
				}
			}
		} else if (next == "allOf") {
			result = resolveIntoArray(result->allOf);
		} else if (next == "anyOf") {
			result = resolveIntoArray(result->anyOf);
		} else if (next == "oneOf") {
			result = resolveIntoArray(result->oneOf);
		} else if (next == "not") {
			result = result->not_;
		} else {
			throw ResolvationError("Attempted to reference a value that is not a schema");
		}
	}
	return result;
}
}
}
}
