#include "JsonPointer.h"

#include <algorithm>

#include "StringUtil.h"
#include "JsonValue.h"

namespace Argonauts {
namespace Util {
namespace Json {
Pointer::Pointer(const std::string &pointer)
{
	// special handling if URI fragment
	bool isURI = false;
	std::size_t nextPos = pointer.find('#');
	if (nextPos == std::string::npos) {
		nextPos = -1; // no fragment, start from the beginning
	} else {
		isURI = true;
	}
	// parse pointer into tokens according to http://tools.ietf.org/html/draft-ietf-appsawg-json-pointer-04#section-3
	while ((nextPos = pointer.find('/', nextPos + 1)) != std::string::npos) {
		const std::size_t next = pointer.find('/', nextPos + 1);
		m_referenceTokens.push_back(pointer.substr(nextPos + 1, next == std::string::npos ? (-1) : (next - 1 - nextPos)));
	}
	// replace ~0 -> ~ and ~1 -> /
	std::transform(m_referenceTokens.begin(), m_referenceTokens.end(), m_referenceTokens.begin(), [](const std::string &str)
	{
		return StringUtil::replaceAll(StringUtil::replaceAll(str, "~0", "~"), "~1", "/");
	});
	if (isURI) {
		std::transform(m_referenceTokens.begin(), m_referenceTokens.end(), m_referenceTokens.begin(), [](std::string &str)
		{
			// TODO: decode percent-encoded values according to http://tools.ietf.org/html/rfc3986#section-2.1
			std::size_t index = 0;
			while ((index = str.find('%', index)) != std::string::npos) {
				const char c = std::stoi(std::string({str[index + 1], str[index + 2]}), nullptr, 16);
				str.replace(index, 3, std::string({c}));
				++index;
			}
			return str;
		});
	}
	m_isValid = true;
}
Pointer::Pointer() : m_isValid(false) {}

const Value Pointer::evaluate(const Value &document) const
{
	if (!isValid()) {
		throw EvaluationError("Cannot evaluate an invalid JSON Pointer");
	}

	Value result = document;
	for (const std::string token : m_referenceTokens) {
		if (result.isObject()) {
			if (result.hasMember(token)) {
				result = result[token];
			} else {
				throw EvaluationError("Cannot evaluate JSON Pointer: token does not exist in object");
			}
		} else if (result.isArray()) {
			try {
				const std::size_t index = std::stoi(token);
				if (index >= result.size()) {
					throw EvaluationError("Cannot evaluate JSON Pointer: array index is out of range");
				}
				result = result[index];
			} catch (std::invalid_argument &e) {
				std::throw_with_nested(EvaluationError("Cannot evaluate JSON Pointer: cannot convert token to integer for array access"));
			}
		} else {
			throw EvaluationError("Cannot evaluate JSON Pointer: cannot recurse into non-container type");
		}
	}
	return result;
}
}
}
}
