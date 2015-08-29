#pragma once

#include <string>

#include "JsonPointer.h"

namespace Argonauts {
namespace Util {
namespace Json {
class Value;

/// Parses and handles a JSON Reference (http://tools.ietf.org/html/draft-pbryan-zyp-json-ref-03)
class Reference
{
	bool m_isValid = false;
	std::string m_ref;
	std::string m_uri;
	Pointer m_pointer;
public:
	explicit Reference(const std::string &ref);
	explicit Reference(const Value &value);
	explicit Reference();

	bool isValid() const { return m_isValid; }
	std::string uri() const { return m_uri; }
	Pointer pointer() const { return m_pointer; }

private:
	std::string extractReference(const Value &value);
};
}
}
}
