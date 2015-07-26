#pragma once

#include <string>

namespace Util
{
void assert(const bool condition, const char *condString, const char *file, const int line, const std::string &message);
std::pair<int, int> lineColumnFromDataAndOffset(const std::string &data, const int offset);
std::string lineInData(const std::string &data, const int offset);
}

#define ASSERT(condition) Util::assert(condition, #condition, __FILE__, __LINE__, std::string());
#define ASSERT_X(condition, message) Util::assert(condition, #condition, __FILE__, __LINE__, message);
