#include "Util.h"

#include <iostream>

void Argonauts::Util::assert(const bool condition, const char *condString, const char *file, const int line, const std::string &message)
{
	if (!condition) {
		std::cerr << "Assertion failed: " << condString << " == false at " << file << ":" << line;
		if (!message.empty()) {
			std::cerr << " (" << message << ")";
		}
		std::cerr << "\n" << std::flush;
		std::abort();
	}
}
