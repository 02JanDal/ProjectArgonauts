#include "Error.h"

#include <string>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>

#include "Util.h"
#include "TermUtil.h"
#include "StringUtil.h"

Error::Error() : std::runtime_error(std::string()) {}
Error::Error(const std::string &msg, const std::size_t offset, const Source &source)
	: std::runtime_error(msg), m_error(msg), m_offset(offset), m_source(source) {}

std::pair<int, int> Error::lineColumnFromDataAndOffset() const
{
	if (m_offset >= m_source.m_data.size())
	{
		return std::make_pair(-1, -1);
	}
	const std::string left = m_source.m_data.substr(0, m_offset); // take the string that preceeds the offset
	const int line = std::count(left.begin(), left.end(), '\n') + 1; // count the number of newlines in that string. this + 1 is the line number
	const int lineStart = left.rfind('\n'); // go back until the last newline in the text before the offset. this is the start of the line
	return std::make_pair(line, m_offset - lineStart); // the column in then simply calculated using the offset and the start of the line
}
std::string Error::lineInData(const int offset) const
{
	const std::string data = m_source.m_data;
	ASSERT(m_offset < data.size());
	int lineStart = data.substr(0, m_offset).rfind('\n');

	const bool isBefore = offset < 0;
	for (int i = 0; i < std::abs(offset); ++i) {
		if (isBefore) {
			lineStart = data.rfind('\n', lineStart - 1);
		} else {
			lineStart = data.find('\n', lineStart + 1);
		}
	}

	lineStart += 1; // it currently is at the \n, we want it after it

	return data.substr(lineStart, data.find('\n', lineStart) - lineStart);
}
std::string Error::errorMessage() const
{
	static const char *TERM_WIDTH = std::getenv("TERM_WIDTH");
	static const int detectedTermWidth = TermUtil::currentWidth();
	static const int termWidth = (TERM_WIDTH == nullptr || std::strcmp(TERM_WIDTH, "") == 0) ? detectedTermWidth : std::stoi(TERM_WIDTH);

	std::string result;

	if (!m_error.empty()) {
		result += m_error;
	}

	const auto position = lineColumnFromDataAndOffset();
	if ((!m_source.m_filename.empty() && m_source.m_filename != "<unknown>") || position != std::make_pair(-1, -1)) {
		result += "\nIn " + m_source.m_filename + ":" + std::to_string(position.first) + ":" + std::to_string(position.second);
	}

	if (!m_source.m_data.empty()) {
		result += '\n';

		const std::string rawLine = lineInData();
		const std::size_t numTabs = std::count(rawLine.begin(), rawLine.end(), '\t');
		const std::string line = StringUtil::replaceAll(rawLine, "\t", "    ");
		if (position.second >= (termWidth - 10)) {
			const std::string l = "..." + line.substr(position.second - (termWidth - 15) + 3, termWidth - 3);
			result += l + '\n'
					+ std::string(3, ' ') + std::string(position.second - 1, ' ').substr(position.second - (termWidth - 15) + 3) + '^';
		} else {
			if (position.first > 0) {
				result += StringUtil::replaceAll(lineInData(-1), "\t", "    ") + '\n';
			}
			result += line + '\n'
					+ std::string(position.second - 1 + numTabs * 3, ' ') + '^';
		}
	}

	return result;
}

Error::Source::Source(const std::string &data, const std::string &filename)
	: m_data(data), m_filename(filename) {}
Error::Source::Source() : Source("") {}
