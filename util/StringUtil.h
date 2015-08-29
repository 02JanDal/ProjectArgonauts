#pragma once

#include <string>
#include <vector>

using StringVector = std::vector<std::string>;

namespace StringUtil
{
std::string charToHexString(const char c);
std::string joinStrings(const StringVector &strings, const std::string &glue);
StringVector splitStrings(const std::string &string, const std::string &delimiter);
std::string replaceAll(const std::string &in, const std::string &match, const std::string &replacement);
std::string toUpper(const std::string &in);
std::string firstLine(const std::string &in);
bool startsWith(const std::string &str, const std::string &match);
bool endsWith(const std::string &str, const std::string &match);
}
