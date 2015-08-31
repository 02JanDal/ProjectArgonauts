#pragma once

#include <string>
#include <vector>

#include "util/Error.h"

namespace Argonauts {
namespace Util {
class SaxSink;

namespace Json {

class SaxReader
{
	// "user accessible" state
	SaxSink *m_handler;
	std::string m_error;
	int m_offset = -1;
public:
	explicit SaxReader(SaxSink *handler);

	void addData(const char *data, const std::size_t size);
	void addData(const std::string &str);
	void end();

	const std::string &errorMessage() const { return m_error; }
	int errorOffset() const { return m_offset; }
	Error error(const std::string &data = "", const std::string &filename = "<unknown>") const { return Error(m_error, std::size_t(m_offset), Error::Source(data, filename)); }
	bool isError() const { return !m_error.empty(); }

private: // internal state
	enum State
	{
		String,
		Special,
		Number,
		Array,
		Object,
		Key,

		WantValue,
		Invalid
	};
	std::vector<State> m_state;
	std::vector<std::size_t> m_arrayObjectSizeStack;

	inline State previousState() const
	{
		if (m_state.size() < 2) {
			return Invalid;
		} else {
			auto it = m_state.begin();
			--it;
			--it;
			return *it;
		}
	}

	char m_itemStart = 0; // the character (", ' or null) that was used to start the current value
	bool m_isEscaped = false;

	inline void appendToCurrentValue(const char c)
	{
		m_currentValue += c;
	}
	inline void resetCurrentValue()
	{
		m_currentValue.clear();
	}

	std::string m_currentValue; // the data of the current string/number/special/key

	void reportError(const std::string &error);
};

class ParseException : public Error
{
	using Error::Error;
};
}
}
}

/// Read chunks of JSON from the given stream to the given sink
std::istream &operator>>(std::istream &stream, Argonauts::Util::SaxSink *handler);
