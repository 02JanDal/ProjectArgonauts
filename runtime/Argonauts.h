#pragma once

#include <stdexcept>
#include <vector>

#include "Parser.h"
#include "Serializer.h"

namespace Argonauts
{
namespace Runtime
{
class Exception : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};
class InvalidEnumValue : public Exception
{
public:
	explicit InvalidEnumValue(const std::string &actual, const std::vector<std::string> &expected);
	explicit InvalidEnumValue(const int actual, const std::vector<int> &expected);
};
class InvalidObjectKey : public Exception
{
public:
	explicit InvalidObjectKey(const std::string &actual, const std::vector<std::string> &expected);
};
}
}
