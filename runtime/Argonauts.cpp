#include "Argonauts.h"

#include <algorithm>

#include "util/StringUtil.h"

namespace Argonauts {
namespace Runtime {
namespace detail {
static std::vector<std::string> intsToStringsHelper(const std::vector<int> &in)
{
	std::vector<std::string> out;
	std::transform(in.begin(), in.end(), std::back_inserter(out), [](const int val) { return std::to_string(val); });
	return out;
}
}

InvalidEnumValue::InvalidEnumValue(const std::string &actual, const std::vector<std::string> &expected)
	: Exception(std::string("Unexpected enum value '") + actual + "', expected one of: '" + Util::String::joinStrings(expected, "', '") + "'") {}
InvalidEnumValue::InvalidEnumValue(const int actual, const std::vector<int> &expected)
	: InvalidEnumValue(std::to_string(actual), detail::intsToStringsHelper(expected)) {}

InvalidObjectKey::InvalidObjectKey(const std::string &actual, const std::vector<std::string> &expected)
	: Exception(std::string("Unexpected object key '") + actual + "', expected one of: '" + Util::String::joinStrings(expected, "', '") + "'") {}

}
}
