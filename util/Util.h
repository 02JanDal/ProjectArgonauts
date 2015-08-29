#pragma once

#include <string>

namespace Argonauts {
namespace Util {
void assert(const bool condition, const char *condString, const char *file, const int line, const std::string &message);
}
}

#define ASSERT(condition) ::Argonauts::Util::assert(condition, #condition, __FILE__, __LINE__, std::string());
#define ASSERT_X(condition, message) ::Argonauts::Util::assert(condition, #condition, __FILE__, __LINE__, message);
