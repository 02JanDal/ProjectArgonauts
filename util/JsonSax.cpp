#include "JsonSax.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

JsonSax::JsonSax(JsonSaxHandler *handler)
	: m_handler(handler), m_state({WantValue})
{
}

static inline bool compareStrings(const char *first, const char *second, const std::size_t size)
{
	return std::strncmp(first, second, size) == 0;
}
static inline bool isValueEnding(const char c)
{
	return c == '}' || c == ']' || c == ',';
}
static inline bool stringContains(const char *str, const std::size_t size, const char c)
{
	for (std::size_t i = 0; i < size; ++i)
	{
		if (str[i] == c)
		{
			return true;
		}
	}
	return false;
}
static inline bool isEscapeCharacter(const char c)
{
	return c == '\\' || c == '/' || c == '"' || c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't';
}
static inline char escapeForCharacter(const char c)
{
	switch (c)
	{
	case '\\': return '\\';
	case '/': return '/';
	case '"': return '"';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	default: return c;
	}
}
static inline bool isWhitespace(const char c)
{
	return c == ' ' || c == '\n' || c == '\t';
}

void JsonSax::addData(const char *data, const std::size_t size)
{
	static const char *validNumberCharacters = "0123456789+-eE.";
	static const std::size_t validNumberCharactersSize = std::strlen(validNumberCharacters);

	for (std::size_t i = 0; i < size; ++i)
	{
		const char next = data[i];
		switch (m_state.back())
		{
		case String:
			if (!m_isEscaped && next == m_itemStart)
			{
				m_handler->string(std::strncpy(new char[m_currentValueSize], m_currentValue, m_currentValueSize), m_currentValueSize);
				resetCurrentValue();
				m_itemStart = 0;
				m_state.pop_back();
			}
			else if (!m_isEscaped && next == '\\')
			{
				m_isEscaped = true;
			}
			else if (m_isEscaped && isEscapeCharacter(next))
			{
				appendToCurrentValue(escapeForCharacter(next));
				m_isEscaped = false;
			}
			else
			{
				appendToCurrentValue(next);
			}
			break;
		case Key:
			// TODO unicode support (\uXXXX)
			if (!m_isEscaped && ((m_itemStart == 0 && next == ':') || next == m_itemStart))
			{
				m_handler->key(std::strncpy(new char[m_currentValueSize], m_currentValue, m_currentValueSize), m_currentValueSize);
				resetCurrentValue();
				m_state.pop_back();
				m_state.push_back(WantValue);
			}
			else if (!m_isEscaped && next == '\\')
			{
				m_isEscaped = true;
			}
			else if (m_isEscaped && isEscapeCharacter(next))
			{
				appendToCurrentValue(escapeForCharacter(next));
			}
			else
			{
				appendToCurrentValue(next);
			}
			break;
		case Special:
			if (isValueEnding(next))
			{
				if (compareStrings(m_currentValue, "null", m_currentValueSize))
				{
					m_handler->null();
				}
				else if (compareStrings(m_currentValue, "false", m_currentValueSize))
				{
					m_handler->boolean(false);
				}
				else if (compareStrings(m_currentValue, "true", m_currentValueSize))
				{
					m_handler->boolean(true);
				}
				else
				{
					char buf[512];
					std::snprintf(buf, 512, "Unexpected %s, expected 'null', 'true' or 'false'", m_currentValue);
					reportError(buf);
					return;
				}
				resetCurrentValue();
				m_state.pop_back();
			}
			else
			{
				appendToCurrentValue(next);
			}
			break;
		case Number:
			if (isValueEnding(next))
			{
				if (stringContains(m_currentValue, m_currentValueSize, '.'))
				{
					m_handler->doubleNumber(std::strtod(m_currentValue, &m_currentValue));
				}
				else
				{
					m_handler->integerNumber(std::strtoll(m_currentValue, &m_currentValue, 10));
				}
				resetCurrentValue();
				m_state.pop_back();
			}
			else if (stringContains(validNumberCharacters, validNumberCharactersSize, next))
			{
				appendToCurrentValue(next);
			}
			else
			{
				char buf[512];
				std::snprintf(buf, 512, "Unexpected %c, expected on of %s", next, validNumberCharacters);
				reportError(buf);
				return;
			}
			break;
		case Object:
			if (next == '"' || next == '\'' || (next >= 'a' && next <= 'z'))
			{
				m_state.push_back(Key);
				if (next != '"' && next != '\'')
				{
					appendToCurrentValue(next);
				}
			}
			else if (next == '}')
			{
				m_handler->endObject(m_arrayObjectSizeStack.back());
				m_arrayObjectSizeStack.pop_back();
				m_state.pop_back();
			}
			else if (next == ',' || isWhitespace(next))
			{
				// no-op
			}
			else
			{
				char buf[512];
				std::snprintf(buf, 512, "Unexpected '%c', expected '\"' or '}'", next);
				reportError(buf);
				return;
			}
			break;
		case Array:
			if (next == ']')
			{
				m_handler->endArray(m_arrayObjectSizeStack.back());
				m_arrayObjectSizeStack.pop_back();
				m_state.pop_back();
				break;
			}
			else if (next == ',')
			{
				break;
			}
		case WantValue:
			switch (next)
			{
			case ':':
				// left over : from key, either double (should we do anything?) or the key had quotes
				// do nothing
				break;
			case '{':
				m_handler->startObject();
				if (m_state.back() != Array)
				{
					m_state.pop_back();
				}
				m_state.push_back(Object);
				m_arrayObjectSizeStack.push_back(0);
				break;
			case '[':
				m_handler->startArray();
				if (m_state.back() != Array)
				{
					m_state.pop_back();
				}
				m_state.push_back(Array);
				m_arrayObjectSizeStack.push_back(0);
				break;
			case '"':
			case '\'':
				if (m_state.size() == 1)
				{
					reportError("String cannot be root entity");
				}
				if (m_state.back() != Array)
				{
					m_state.pop_back();
				}
				m_state.push_back(String);
				m_itemStart = next;
				break;
			case '-':
			case '+':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (m_state.size() == 1)
				{
					reportError("Number cannot be root entity");
				}
				if (m_state.back() != Array)
				{
					m_state.pop_back();
				}
				m_state.push_back(Number);
				appendToCurrentValue(next);
				break;
			case 'n':
			case 't':
			case 'f':
				if (m_state.size() == 1)
				{
					reportError("Keyword cannot be root entity");
				}
				if (m_state.back() != Array)
				{
					m_state.pop_back();
				}
				m_state.push_back(Special);
				appendToCurrentValue(next);
				break;
			case '\n':
			case '\t':
			case ' ':
				// no-op
				break;
			default:
				char buf[512];
				std::snprintf(buf, 512, "Unexpected '%c', expected VALUE", next);
				reportError(buf);
				return;
			}
		}
	}
}
void JsonSax::addData(const std::string &str)
{
	return addData(str.c_str(), str.size());
}

void JsonSax::end()
{
	if (!m_state.empty())
	{
		char buf[512];
		switch (m_state.back())
		{
		case String:
			std::snprintf(buf, 512, "Unexpected EOF, expected %c", m_itemStart);
			break;
		case Number:
			switch (previousState())
			{
			case Object:
				std::snprintf(buf, 512, "Unexpected EOF, expected '}' or ','");
				break;
			case Array:
				std::snprintf(buf, 512, "Unexpected EOF, expected ']' or ','");
				break;
			default:
				std::snprintf(buf, 512, "Unexpected EOF");
			}
			break;
		case Key:
			std::snprintf(buf, 512, "Unexpected EOF, expected ':'");
			break;
		case Object:
			std::snprintf(buf, 512, "Unexpected EOF, expected '}' or ','");
			break;
		case Array:
			std::snprintf(buf, 512, "Unexpected EOF, expected ']' or ','");
			break;
		case Special:
			std::snprintf(buf, 512, "Unexpected EOF, expected CHARACTER");
			break;
		default:
			std::snprintf(buf, 512, "Unexpected EOF");
		}
		reportError(buf);
	}
}

void JsonSax::reportError(char *error)
{
	m_error = error;
}
void JsonSax::reportError(const char *error)
{
	char str[std::strlen(error)];
	m_error = std::strcpy(str, error);
}
