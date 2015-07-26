#pragma once

#include <string>
#include <vector>
#include <functional>

#include "util/ArgonautsException.h"
#include "util/SelfContainerIterator.h"

class Lexer
{
public:
	explicit Lexer();

	class LexerException : public ArgonautsException { using ArgonautsException::ArgonautsException; };
	class UnexpectedCharacterException : public LexerException
	{
	public:
		explicit UnexpectedCharacterException(const std::string &message, const int offset)
			: LexerException(message), offset(offset) {}

		const int offset;
	};

	struct Token
	{
		explicit Token() : type(Invalid) {}

		enum Type
		{
			Identifier,
			String,
			Integer,

			Keyword_Enum,
			Keyword_Struct,

			ExclamationMark,
			AtSymbol,
			ParanthesisOpen,
			ParanthesisClose,
			CurlyBracketOpen,
			CurlyBracketClose,
			AngleBracketOpen,
			AngleBracketClose,
			SemiColon,
			Comma,
			Equal,
			EndOfFile,

			Invalid
		} type;

		std::string string;
		int64_t integer;

		int offset;

		static const std::string toString(const Type type);

		static Token createIdentifier(const std::string &name, const int offset) { return Token(Identifier, name, 0, offset); }
		static Token createString(const std::string &string, const int offset) { return Token(String, string, 0, offset); }
		static Token createInteger(const int64_t &integer, const int offset) { return Token(Integer, std::string(), integer, offset); }
		static Token createSpecial(const Type type, const int offset) { return Token(type, std::string(), 0, offset); }

	private:
		explicit Token(const Type &type, const std::string &string, const int64_t integer, const int offset)
			: type(type), string(string), integer(integer), offset(offset) {}
	};

	std::vector<Token> consume(const std::string &data);

private:
	std::string consumeWhile(SelfContainedIterator<std::string> &it, const std::function<bool(const char)> &isAcceptedCallback);

	enum CharacterClass
	{
		Letter,
		Digit,
		Space,
		Other
	};
	static const CharacterClass classifyCharacter(const char c);
};
