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

#include "util/StringUtil.h"
#include "util/Error.h"

namespace Argonauts {
namespace Tool {

static std::string cleanString(const std::string &in)
{
	using Util::String::replaceAll;
	return replaceAll(replaceAll(replaceAll(in, "\\\"", "\""), "\\\n", "\n"), "\\\t", "\t");
}

Lexer::Lexer()
{
}

std::vector<Lexer::Token> Lexer::consume(const std::string &data, const std::string &filename)
{
	std::vector<Token> tokens;
	Util::SelfContainedIterator<std::string> it(data);
	while (it.hasNext())
	{
		const char next = it.next();
		const int offset = it.index();
		if (classifyCharacter(next) == Digit || (next == '-' && classifyCharacter(it.peekNext()) == Digit)) {
			const std::string str = next + consumeWhile(it, [](const char c) -> bool { return classifyCharacter(c) == Digit; });
			tokens.push_back(Token::createInteger(std::stoll(str), offset));
		} else if (classifyCharacter(next) == Letter) {
			const std::string string = next + consumeWhile(it, [](const char c) -> bool
			{
				const CharacterClass cc = classifyCharacter(c);
				return cc == Letter || cc == Digit || c == '_';
			});
			if (string == "enum") {
				tokens.push_back(Token::createSpecial(Token::Keyword_Enum, offset));
			} else if (string == "struct") {
				tokens.push_back(Token::createSpecial(Token::Keyword_Struct, offset));
			} else if (string == "using") {
				tokens.push_back(Token::createSpecial(Token::Keyword_Using, offset));
			} else {
				tokens.push_back(Token::createIdentifier(string, offset));
			}
		} else if (next == '/' && it.peekNext() == '/') {
			consumeWhile(it, [](const char c) -> bool { return c != '\n'; });
		} else {
			switch (next) {
			case '"': {
				bool isEscape = false;
				tokens.push_back(Token::createString(cleanString(consumeWhile(it, [&isEscape](const char c) -> bool
				{
					if (c == '"' && !isEscape)
					{
						return false;
					}
					else if (c == '\\' && !isEscape)
					{
						isEscape = true;
					}
					else
					{
						isEscape = false;
					}
					return true;
				})), offset));
				it.next(); // ending "
				break;
			}
			case '!':
				tokens.push_back(Token::createSpecial(Token::ExclamationMark, offset));
				break;
			case '@':
				tokens.push_back(Token::createSpecial(Token::AtSymbol, offset));
				break;
			case '(':
				tokens.push_back(Token::createSpecial(Token::ParanthesisOpen, offset));
				break;
			case ')':
				tokens.push_back(Token::createSpecial(Token::ParanthesisClose, offset));
				break;
			case '{':
				tokens.push_back(Token::createSpecial(Token::CurlyBracketOpen, offset));
				break;
			case '}':
				tokens.push_back(Token::createSpecial(Token::CurlyBracketClose, offset));
				break;
			case '<':
				tokens.push_back(Token::createSpecial(Token::AngleBracketOpen, offset));
				break;
			case '>':
				tokens.push_back(Token::createSpecial(Token::AngleBracketClose, offset));
				break;
			case ';':
				tokens.push_back(Token::createSpecial(Token::SemiColon, offset));
				break;
			case ':':
				tokens.push_back(Token::createSpecial(Token::Colon, offset));
				break;
			case ',':
				tokens.push_back(Token::createSpecial(Token::Comma, offset));
				break;
			case '.':
				tokens.push_back(Token::createSpecial(Token::Dot, offset));
				break;
			case '=':
				tokens.push_back(Token::createSpecial(Token::Equal, offset));
				break;
			case '\n':
			case '\r':
			case ' ':
			case '\t':
				break;
			default:
				throw Util::Error(std::string("Unexpected character: ") + next + " (0x" + Util::String::charToHexString(next) + ")", offset, Util::Error::Source(data, filename));
			}
		}
	}
	tokens.push_back(Token::createSpecial(Token::EndOfFile, data.size()));
	return tokens;
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

const std::string Lexer::Token::toString(const Type type)
{
	switch (type)
	{
	case Identifier: return "IDENTIFIER";
	case String: return "STRING";
	case Integer: return "INTEGER";
	case Keyword_Enum: return "ENUM";
	case Keyword_Struct: return "STRUCT";
	case Keyword_Using: return "USING";
	case ExclamationMark: return "EXCLAMATION_MARK";
	case AtSymbol: return "AT_SYMBOL";
	case ParanthesisOpen: return "PARANTHESIS_OPEN";
	case ParanthesisClose: return "PRARANTHESIS_CLOSE";
	case CurlyBracketOpen: return "CURLY_BRACKET_OPEN";
	case CurlyBracketClose: return "CURLY_BRACKET_CLOSE";
	case AngleBracketOpen: return "ANGLE_BRACKET_OPEN";
	case AngleBracketClose: return "ANGLE_BRACKET_CLOSE";
	case SemiColon: return "SEMICOLON";
	case Comma: return "COMMA";
	case Colon: return "COLON";
	case Dot: return "DOT";
	case Equal: return "EQUAL";
	case EndOfFile: return "EOF";
	case Invalid: return "INVALID";
	}
}

}
}
