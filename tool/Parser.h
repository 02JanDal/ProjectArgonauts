#pragma once

#include <vector>

#include "util/ArgonautsException.h"
#include "util/SelfContainerIterator.h"
#include "Lexer.h"

namespace Argonauts
{
class File;
class Struct;
class Enum;
class Annotation;
}

class Parser
{
	using Token = Lexer::Token;
	using Iterator = SelfContainedIterator<std::vector<Token>>;

	Iterator it;
public:
	explicit Parser(const std::vector<Lexer::Token> &tokens);

	class ParserException : public ArgonautsException
	{
	public:
		explicit ParserException(const std::string &what = std::string(), const int offset = 0)
			: ArgonautsException(what), offset(offset) {}

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

	Argonauts::File process();

private:
	Argonauts::Struct consumeStruct(const std::vector<Argonauts::Annotation> &annotations);
	Argonauts::Enum consumeEnum(const std::vector<Argonauts::Annotation> &annotations);
	Argonauts::Annotation consumeAnnotation();

	inline Lexer::Token consumeToken(const Token::Type type) { return consumeToken({type}); }
	Lexer::Token consumeToken(const std::initializer_list<Token::Type> &types = {Token::Invalid});
};
