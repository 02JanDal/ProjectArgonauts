#include "StringUtil.h"

#include <iostream>
#include <algorithm>

std::string StringUtil::charToHexString(const char c)
{
	static const char hexChars[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	std::string out;
	out += hexChars[(c & 0xF0) >> 4];
	out += hexChars[(c & 0x0F) >> 0];
	return out;
}

std::string StringUtil::joinStrings(const StringVector &strings, const std::string &glue)
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
	size_t pos = 0;
	while ((pos = result.find(match, pos)) != std::string::npos)
	{
		result.replace(pos, match.size(), replacement);
		pos += replacement.size();
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

StringVector StringUtil::splitStrings(const std::string &string, const std::string &delimiter)
{
	if (string.empty())
	{
		return {};
	}

	std::size_t index = string.find(delimiter);
	if (index == std::string::npos)
	{
		return {string};
	}

	std::size_t prevIndex = 0;
	std::vector<std::string> out;
	while (index != std::string::npos)
	{
		out.push_back(string.substr(prevIndex, index - prevIndex));
		prevIndex = index + 1;
		index = string.find(delimiter, index + 1);
	}
	out.push_back(string.substr(prevIndex));

	return out;
}

bool StringUtil::startsWith(const std::string &str, const std::string &match)
{
	return str.find(match) == 0;
}
bool StringUtil::endsWith(const std::string &str, const std::string &match)
{
	return str.rfind(match) == match.size();
}
