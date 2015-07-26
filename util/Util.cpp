#include "Util.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>

void Util::assert(const bool condition, const char *condString, const char *file, const int line, const std::string &message)
{
	if (!condition)
	{
		std::cerr << "Assertion failed: " << condString << " == false at " << file << ":" << line;
		if (!message.empty())
		{
			std::cerr << " (" << message << ")";
		}
		std::cerr << "\n" << std::flush;
		std::abort();
	}
}


std::pair<int, int> Util::lineColumnFromDataAndOffset(const std::string &data, const int offset)
{
	if (offset >= data.size())
	{
		return std::make_pair(-1, -1);
	}
	const std::string left = data.substr(0, offset); // take the string that preceeds the offset
	const int line = std::count(left.begin(), left.end(), '\n') + 1; // count the number of newlines in that string. this + 1 is the line number
	const int lineStart = left.rfind('\n'); // go back until the last newline in the text before the offset. this is the start of the line
	return std::make_pair(line, offset - lineStart); // the column in then simply calculated using the offset and the start of the line
}


std::string Util::lineInData(const std::string &data, const int offset)
{
	ASSERT(offset < data.size());
	const std::string left = data.substr(0, offset);
	const int lineStart = left.rfind('\n') + 1;
	return data.substr(lineStart, data.find('\n', lineStart) - lineStart);
}
