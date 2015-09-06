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

#include "Lexer.h"

#include <algorithm>

#include "util/StringUtil.h"
#include "util/Error.h"
#include "Token.h"

namespace Argonauts {
namespace Common {

static std::string cleanString(const std::string &in)
{
	using Util::String::replaceAll;
	return replaceAll(replaceAll(replaceAll(in, "\\\"", "\""), "\\\n", "\n"), "\\\t", "\t");
}

Lexer::Lexer()
{
}

std::vector<Token> Lexer::consume(const std::string &data, const std::string &filename, const bool isEnd)
{
	const int startOffset = m_offset;
	std::vector<Token> tokens;
	Util::SelfContainedIterator<std::string> it(data);

	auto handleString = [this, &it, &tokens]()
	{
		bool isEscape = false;
		const std::string string = cleanString(consumeWhile(it, [&isEscape](const char c) -> bool
		{
			if (c == '"' && !isEscape) {
				return false;
			} else if (c == '\\' && !isEscape) {
				isEscape = true;
			} else {
				isEscape = false;
			}
			return true;
		}));
		tokens.push_back(Token::createString(string, m_offset, string.size() + (m_state == InString ? 0 : 1) + (it.hasNext() ? 1 : 0)));
		if (!it.hasNext()) {
			m_state = InString;
		} else {
			m_state = Normal;
			it.next(); // ending "
		}
	};
	if (m_state == InString) {
		handleString();
	}

	while (it.hasNext())
	{
		const char next = it.next();
		m_offset = startOffset + it.index();
		if (classifyCharacter(next) == Digit || (next == '-' && classifyCharacter(it.peekNext()) == Digit)) {
			const std::string str = next + consumeWhile(it, [](const char c) -> bool { return classifyCharacter(c) == Digit; });
			tokens.push_back(Token::createInteger(std::stoll(str), m_offset, str.size()));
		} else if (classifyCharacter(next) == Letter) {
			const std::string string = next + consumeWhile(it, [](const char c) -> bool
			{
				const CharacterClass cc = classifyCharacter(c);
				return cc == Letter || cc == Digit || c == '_' || c == '.';
			});
			if (string == "enum") {
				tokens.push_back(Token::createSpecial(Token::Keyword_Enum, m_offset, string.size()));
			} else if (string == "struct") {
				tokens.push_back(Token::createSpecial(Token::Keyword_Struct, m_offset, string.size()));
			} else if (string == "using") {
				tokens.push_back(Token::createSpecial(Token::Keyword_Using, m_offset, string.size()));
			} else {
				tokens.push_back(Token::createIdentifier(string, m_offset));
			}
		} else if (next == '/' && it.peekNext() == '/') {
			tokens.push_back(Token::createComment(consumeWhile(it, [](const char c) -> bool { return c != '\n'; }), m_offset));
		} else {
			switch (next) {
			case '"':
				handleString();
				break;
			case '!':
				tokens.push_back(Token::createSpecial(Token::ExclamationMark, m_offset));
				break;
			case '@':
				tokens.push_back(Token::createSpecial(Token::AtSymbol, m_offset));
				break;
			case '(':
				tokens.push_back(Token::createSpecial(Token::ParanthesisOpen, m_offset));
				break;
			case ')':
				tokens.push_back(Token::createSpecial(Token::ParanthesisClose, m_offset));
				break;
			case '{':
				tokens.push_back(Token::createSpecial(Token::CurlyBracketOpen, m_offset));
				break;
			case '}':
				tokens.push_back(Token::createSpecial(Token::CurlyBracketClose, m_offset));
				break;
			case '<':
				tokens.push_back(Token::createSpecial(Token::AngleBracketOpen, m_offset));
				break;
			case '>':
				tokens.push_back(Token::createSpecial(Token::AngleBracketClose, m_offset));
				break;
			case ';':
				tokens.push_back(Token::createSpecial(Token::SemiColon, m_offset));
				break;
			case ':':
				tokens.push_back(Token::createSpecial(Token::Colon, m_offset));
				break;
			case ',':
				tokens.push_back(Token::createSpecial(Token::Comma, m_offset));
				break;
			case '=':
				tokens.push_back(Token::createSpecial(Token::Equal, m_offset));
				break;
			case '\n':
			case '\r':
			case ' ':
			case '\t':
				break;
			default:
				tokens.push_back(Token::createError(std::string("Unexpected character: ") + next + " (0x" + Util::String::charToHexString(next) + ")", m_offset, 1));
			}
		}
	}
	if (isEnd) {
		tokens.push_back(Token::createSpecial(Token::EndOfFile, m_offset + 1, 0));
	}
	return tokens;
}

std::vector<Token> Lexer::cleanComments(const std::vector<Token> &tokens)
{
	std::vector<Token> out;
	std::copy_if(tokens.begin(), tokens.end(), std::back_inserter(out), [](const Token &token) { return token.type != Token::Comment; });
	return out;
}

std::string Lexer::consumeWhile(Util::SelfContainedIterator<std::string> &it, const std::function<bool (const char)> &isAcceptedCallback)
{
	std::string out;
	while (it.hasNext() && isAcceptedCallback(it.peekNext()))
	{
		out += it.next();
	}
	return out;
}

Lexer::CharacterClass Lexer::classifyCharacter(const char c)
{
	if (c >= '0' && c <= '9')
	{
		return Digit;
	}
	else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
	{
		return Letter;
	}
	else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
	{
		return Space;
	}
	else
	{
		return Other;
	}
}

}
}
