#include "JsonReference.h"

#include "JsonValue.h"
#include "Json.h"

namespace Argonauts {
namespace Util {
namespace Json {
Reference::Reference(const std::string &ref)
	: m_ref(ref)
{
	const std::size_t pound = ref.find('#');
	if (pound != std::string::npos) {
		m_uri = ref.substr(0, pound);
		m_pointer = Pointer(ref.substr(pound + 1));

		m_isValid = (m_uri.empty() || isURI(m_uri)) && m_pointer.isValid();
	}
}
Reference::Reference(const Value &value)
	: Reference(extractReference(value))
{
}
Reference::Reference() {}

std::string Reference::extractReference(const Value &value)
{
	if (value.isString()) {
		return value.toString();
	} else if (value.isObject()) {
		if (value.hasMember("$ref") && value["$ref"].isString()) {
			return value["$ref"].toString();
		}
	}
	return std::string();
}
}
}
}
