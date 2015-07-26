#include "StringUtil.h"

#include <iostream>
#include <algorithm>

std::string StringUtil::charToHexString(const char c)
{
	std::string out;
	out += ((c & 0xF0) >> 4) + '0';
	out += ((c & 0xF0) >> 0) + '0';
	return out;
}

std::string StringUtil::joinStrings(const std::vector<std::string> &strings, const std::string &glue)
{
	if (strings.size() == 0)
	{
		return std::string();
	}
	else if (strings.size() == 1)
	{
		return strings.front();
	}
	else
	{
		std::vector<std::string> stringsCopy(strings);
		const std::string last = stringsCopy.back(); stringsCopy.pop_back();
		std::string result;
		for (const std::string &string : stringsCopy)
		{
			result += string + glue;
		}
		result += last;
		return result;
	}
}

std::string StringUtil::replaceAll(const std::string &in, const std::string &match, const std::string &replacement)
{
	std::string result = in;
	size_t start;
	while ((start = result.find(match)) != std::string::npos)
	{
		result.replace(start, match.size(), replacement);
	}
	return result;
}


std::string StringUtil::toUpper(const std::string &in)
{
	std::string out;
	std::transform(in.begin(), in.end(), std::back_inserter(out), [](const char c) { return std::toupper(c); });
	return out;
}

std::string StringUtil::firstLine(const std::string &in)
{
	const size_t lineEndingPos = in.find('\n');
	if (lineEndingPos == std::string::npos)
	{
		return in;
	}
	else
	{
		return in.substr(0, lineEndingPos);
	}
}
