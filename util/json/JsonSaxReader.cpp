/*
 * Copyright 2015 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "JsonSaxReader.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <istream>

#include "util/Util.h"
#include "util/SaxSink.h"

namespace Argonauts {
namespace Util {
namespace Json {
SaxReader::SaxReader(SaxSink *handler)
	: m_handler(handler), m_state({WantValue})
{
}

static inline bool isValueEnding(const char c)
{
	return c == '}' || c == ']' || c == ',' || std::isspace(c);
}
static inline bool stringContains(const char *str, const std::size_t size, const char c)
{
	for (std::size_t i = 0; i < size; ++i) {
		if (str[i] == c) {
			return true;
		}
	}
	return false;
}
static inline bool isEscapeCharacter(const char c)
{
	return c == '\\' || c == '/' || c == '"' || c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't' || c == 'u';
}
static inline char escapeForCharacter(const char c)
{
	switch (c) {
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

#define SEND_TO_HANDLER0(NAME) if (!m_handler->NAME()) { ASSERT(!m_handler->error().empty()); reportError(m_handler->error()); return; }
#define SEND_TO_HANDLER1(NAME, ARG) if (!m_handler->NAME(ARG)) { ASSERT(!m_handler->error().empty()); reportError(m_handler->error()); return; }
void SaxReader::addData(const char *data, const std::size_t size)
{
	static const char *validNumberCharacters = "0123456789+-eE.";
	static const std::size_t validNumberCharactersSize = std::strlen(validNumberCharacters);

	if (isError()) {
		return;
	}

	for (std::size_t i = 0; i < size; ++i)
	{
		++m_offset;
		const char next = data[i];
		switch (m_state.back()) {
		case String:
		case Key:
			if (!m_utfString.empty()) {
				m_utfString += next;
				if (m_utfString.size() == 5) {
					appendToCurrentValue(std::stoi(m_utfString, 0, 16));
				}
			} else if (!m_isEscaped && ((m_itemStart == 0 && next == ':') || next == m_itemStart)) {
				if (m_state.back() == String) {
					SEND_TO_HANDLER1(string, m_currentValue);
				} else {
					SEND_TO_HANDLER1(key, m_currentValue);
				}
				resetCurrentValue();
				m_itemStart = 0;
				if (m_state.back() == Key) {
					m_state.pop_back();
					m_state.push_back(WantValue);
				} else {
					m_state.pop_back();
				}
			} else if (!m_isEscaped && next == '\\') {
				m_isEscaped = true;
			} else if (m_isEscaped) {
				if (isEscapeCharacter(next)) {
					appendToCurrentValue(escapeForCharacter(next));
				} else if (next == 'u') {
					m_utfString = "u";
				} else {
					appendToCurrentValue(next);
				}
				m_isEscaped = false;
			} else {
				appendToCurrentValue(next);
			}
			break;
		case Special:
			if (isValueEnding(next)) {
				if (m_currentValue == "null") {
					SEND_TO_HANDLER0(null);
				} else if (m_currentValue == "false") {
					SEND_TO_HANDLER1(boolean, false);
				} else if (m_currentValue == "true") {
					SEND_TO_HANDLER1(boolean, true);
				} else {
					reportError("Unexpected " + m_currentValue + ", expected 'null', 'true' or 'false'");
					return;
				}
				resetCurrentValue();
				m_state.pop_back();
			} else {
				appendToCurrentValue(next);
			}
			break;
		case Number:
			if (isValueEnding(next)) {
				if (m_currentValue.find('.') != std::string::npos) {
					SEND_TO_HANDLER1(doubleNumber, std::stod(m_currentValue));
				} else {
					SEND_TO_HANDLER1(integerNumber, std::stoll(m_currentValue));
				}
				resetCurrentValue();
				m_state.pop_back();
			} else if (stringContains(validNumberCharacters, validNumberCharactersSize, next)) {
				appendToCurrentValue(next);
			} else {
				reportError(std::string("Unexpected ") + next + ", expected one of " + validNumberCharacters);
				return;
			}
			break;
		case Object:
			if (next == '"' || next == '\'' || (next >= 'a' && next <= 'z')) {
				m_state.push_back(Key);
				if (next != '"' && next != '\'') {
					appendToCurrentValue(next);
					m_itemStart = 0;
				} else {
					m_itemStart = next;
				}
				m_arrayObjectSizeStack.back() += 1;
			} else if (next == '}') {
				SEND_TO_HANDLER1(endObject, m_arrayObjectSizeStack.back());
				m_arrayObjectSizeStack.pop_back();
				m_state.pop_back();
			} else if (next == ',' || std::isspace(next)) {
				// no-op
			} else {
				reportError(std::string("Unexpected '") + next + "', expected '\"' or '}'");
				return;
			}
			break;
		case Array:
			if (next == ']') {
				SEND_TO_HANDLER1(endArray, m_arrayObjectSizeStack.back());
				m_arrayObjectSizeStack.pop_back();
				m_state.pop_back();
				break;
			} else if (next == ',') {
				break;
			}
			m_arrayObjectSizeStack.back() += 1;
			// intentional fall-through
			[[clang::fallthrough]];
		case WantValue:
			switch (next) {
			case ':':
				// left over : from key, either double (should we do anything?) or the key had quotes
				// do nothing
				break;
			case '{':
				SEND_TO_HANDLER0(startObject);
				if (m_state.back() != Array) {
					m_state.pop_back();
				}
				m_state.push_back(Object);
				m_arrayObjectSizeStack.push_back(0);
				break;
			case '[':
				SEND_TO_HANDLER0(startArray);
				if (m_state.back() != Array) {
					m_state.pop_back();
				}
				m_state.push_back(Array);
				m_arrayObjectSizeStack.push_back(0);
				break;
			case '"':
			case '\'':
				if (m_state.size() == 1) {
					reportError("String cannot be root entity");
				}
				if (m_state.back() != Array) {
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
				if (m_state.size() == 1) {
					reportError("Number cannot be root entity");
				}
				if (m_state.back() != Array) {
					m_state.pop_back();
				}
				m_state.push_back(Number);
				appendToCurrentValue(next);
				break;
			case 'n':
			case 't':
			case 'f':
				if (m_state.size() == 1) {
					reportError("Keyword cannot be root entity");
				}
				if (m_state.back() != Array) {
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
				reportError(std::string("Unexpected '") + next + "', expected VALUE");
				return;
			}
			break;
		case Invalid:
			ASSERT(false);
		}
	}
}
void SaxReader::addData(const std::string &str)
{
	return addData(str.c_str(), str.size());
}

void SaxReader::end()
{
	if (isError()) {
		return;
	}

	if (!m_state.empty()) {
		switch (m_state.back()) {
		case String:
			reportError(std::string("Unexpected EOF, expected '") + m_itemStart + '\'');
			break;
		case Number:
			switch (previousState()) {
			case Object:
				reportError("Unexpected EOF, expected '}' or ','");
				break;
			case Array:
				reportError("Unexpected EOF, expected ']' or ','");
				break;
			case String:
			case Key:
			case Number:
			case Special:
			case WantValue:
			case Invalid:
				reportError("Unexpected EOF");
			}
			break;
		case Key:
			reportError("Unexpected EOF, expected ':'");
			break;
		case Object:
			reportError("Unexpected EOF, expected '}' or ','");
			break;
		case Array:
			reportError("Unexpected EOF, expected ']' or ','");
			break;
		case Special:
			reportError("Unexpected EOF, expected CHARACTER");
			break;
		case WantValue:
			reportError("Unexpected EOF, expected '{', '[', '\"', NUMBER, 'true', 'false' or null'");
			break;
		case Invalid:
			reportError("Unexpected EOF");
		}
	}
}

void SaxReader::reportError(const std::string &error)
{
	m_error = error;
}
}

}
}

std::istream &operator>>(std::istream &stream, Argonauts::Util::SaxSink *handler)
{
	static const std::size_t chunkSize = 512;
	char buffer[chunkSize];

	Argonauts::Util::Json::SaxReader reader(handler);

	while (!stream.good() && !reader.isError()) {
		const auto size = stream.readsome(buffer, chunkSize);
		if (size == -1) {
			throw Argonauts::Util::Error("Unable to read from input stream", std::size_t(reader.errorOffset()));
		}
		reader.addData(buffer, std::size_t(size));
	}

	reader.end();

	if (reader.isError()) {
		throw reader.error();
	}

	return stream;
}
