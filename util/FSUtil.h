#pragma once

#include <string>

namespace Argonauts {
namespace Util {
namespace FS {

std::string readFile(const std::string &filename);
void writeFile(const std::string &filename, const std::string &data);

}
}
}
