#pragma once

#include <string>
#include <vector>
#include <cstdlib>

#include "SelfContainerIterator.h"

class JsonSaxHandler
{
public:
	virtual bool null() = 0;
	virtual bool boolean(const bool) = 0;
	virtual bool integerNumber(const int64_t) = 0;
	virtual bool doubleNumber(const double) = 0;
	virtual bool string(const char *str, const std::size_t size) = 0;
	virtual bool startObject() = 0;
	virtual bool key(const char *str, const std::size_t size) = 0;
	virtual bool endObject(const std::size_t size) = 0;
	virtual bool startArray() = 0;
	virtual bool endArray(const std::size_t size) = 0;
};

class JsonSax
{
	using Iterator = SelfContainedIterator<char>;

	JsonSaxHandler *m_handler;
	char *m_error = nullptr;
public:
	explicit JsonSax(JsonSaxHandler *handler);

	void addData(const char *data, const std::size_t size);
	void addData(const std::string &str);
	void end();

	/// Ownership of the returned string is kept
	const char *error() const { return m_error; }
	bool isError() const { return m_error != nullptr; }

private:
	// current state
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
	State m_previous = Invalid;
	std::vector<State> m_state;
	std::vector<std::size_t> m_arrayObjectSizeStack;

	inline State previousState() const
	{
		if (m_state.size() < 2)
		{
			return Invalid;
		}
		else
		{
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
		// expand the buffer if needed
		if (m_currentValueSize >= m_currentValueBufferSize)
		{
			if (m_currentValueBufferSize == 0)
			{
				m_currentValue = (char *)std::malloc(64);
				m_currentValueBufferSize = 64;
			}
			else
			{
				m_currentValue = (char *)std::realloc(m_currentValue, m_currentValueBufferSize += 64);
			}
		}
		m_currentValue[m_currentValueSize++] = c;
	}
	inline void resetCurrentValue()
	{
		if (m_currentValueBufferSize >= 512)
		{
			m_currentValue = (char *)std::realloc(m_currentValue, 64);
			m_currentValueBufferSize = 64;
		}
		m_currentValueSize = 0;
	}

	char *m_currentValue; // the data of the current string/number/special/key
	std::size_t m_currentValueSize = 0;
	std::size_t m_currentValueBufferSize = 0;

	/// Report an error. Takes ownership of error
	void reportError(char *error);
	/// Report an error. Does not take ownership of error
	void reportError(const char *error);
};
