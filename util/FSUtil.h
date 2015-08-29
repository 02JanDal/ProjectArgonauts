#pragma once

#include <string>

namespace FSUtil
{
std::string readFile(const std::string &filename);
void writeFile(const std::string &filename, const std::string &data);
}
