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

#pragma once

#include <vector>

#include "util/ArgonautsException.h"
#include "util/SelfContainerIterator.h"
#include "Lexer.h"
#include "DataTypes.h"

namespace Argonauts {
namespace Tool {
class Parser
{
	using Token = Lexer::Token;
	using Iterator = Util::SelfContainedIterator<std::vector<Token>>;

	Iterator it;
public:
	explicit Parser(const std::vector<Lexer::Token> &tokens);

	class ParserException : public Util::Exception
	{
	public:
		explicit ParserException(const std::string &what = std::string(), const int offset_ = 0)
			: Util::Exception(what), offset(offset_) {}

		const int offset;
	};
	class UnexpectedEndOfTokenStreamException : public ParserException
	{
	public:
		explicit UnexpectedEndOfTokenStreamException() : ParserException("Unexpected end of token stream") {}
	};
	class UnexpectedTokenException : public ParserException
	{
	public:
		using ParserException::ParserException;

		explicit UnexpectedTokenException(const Token &actual, const std::initializer_list<Token::Type> &expected)
			: ParserException(message(actual, expected), actual.offset) {}

	private:
		static std::string message(const Token &actual, const std::initializer_list<Token::Type> &expected);
	};

	File process();

private:
	Struct consumeStruct(const Annotations &annotations);
	Enum consumeEnum(const Annotations &annotations);
	std::unordered_multimap<PositionedString, Annotations::Value> consumeAnnotation();
	Type::Ptr consumeType();
	PositionedString consumeAttributeName();

	inline Lexer::Token consumeToken(const Token::Type type) { return consumeToken({type}); }
	Lexer::Token consumeToken(const std::initializer_list<Token::Type> &types = {Token::Invalid});
};
}
}
