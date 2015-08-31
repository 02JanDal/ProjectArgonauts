#pragma once

#include <string>
#include <vector>
#include <functional>

#include "util/SelfContainerIterator.h"

namespace Argonauts {
namespace Tool {

class Lexer
{
public:
	explicit Lexer();

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
			Keyword_Using,

			ExclamationMark,
			AtSymbol,
			ParanthesisOpen,
			ParanthesisClose,
			CurlyBracketOpen,
			CurlyBracketClose,
			AngleBracketOpen,
			AngleBracketClose,
			SemiColon,
			Colon,
			Comma,
			Dot,
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
		explicit Token(const Type &type_, const std::string &string_, const int64_t integer_, const int offset_)
			: type(type_), string(string_), integer(integer_), offset(offset_) {}
	};

	std::vector<Token> consume(const std::string &data, const std::string &filename);

private:
	std::string consumeWhile(Util::SelfContainedIterator<std::string> &it, const std::function<bool(const char)> &isAcceptedCallback);

	enum CharacterClass
	{
		Letter,
		Digit,
		Space,
		Other
	};
	static CharacterClass classifyCharacter(const char c);
};

}
}
