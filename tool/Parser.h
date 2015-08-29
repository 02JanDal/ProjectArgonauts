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
	using Iterator = SelfContainedIterator<std::vector<Token>>;

	Iterator it;
public:
	explicit Parser(const std::vector<Lexer::Token> &tokens);

	class ParserException : public ArgonautsException
	{
	public:
		explicit ParserException(const std::string &what = std::string(), const int offset_ = 0)
			: ArgonautsException(what), offset(offset_) {}

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
	std::unordered_map<std::string, Annotations::Value> consumeAnnotation();
	Type::Ptr consumeType();

	inline Lexer::Token consumeToken(const Token::Type type) { return consumeToken({type}); }
	Lexer::Token consumeToken(const std::initializer_list<Token::Type> &types = {Token::Invalid});
};
}
}
