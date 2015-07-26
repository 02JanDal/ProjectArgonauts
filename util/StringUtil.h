#pragma once

#include <string>
#include <vector>

namespace StringUtil
{
std::string charToHexString(const char c);
std::string joinStrings(const std::vector<std::string> &strings, const std::string &glue);
std::string replaceAll(const std::string &in, const std::string &match, const std::string &replacement);
std::string toUpper(const std::string &in);
std::string firstLine(const std::string &in);
}
