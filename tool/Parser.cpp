#include "Parser.h"

#include <algorithm>

#include "util/Util.h"
#include "util/StringUtil.h"
#include "DataTypes.h"

namespace Argonauts {
namespace Tool {

Parser::Parser(const std::vector<Lexer::Token> &tokens)
	: it(tokens)
{
}

File Parser::process()
{
	File file;
	Annotations annotations;
	while (it.hasNext())
	{
		const Token next = consumeToken({Token::AtSymbol, Token::Keyword_Enum, Token::Keyword_Struct, Token::EndOfFile});
		switch (next.type)
		{
		case Token::AtSymbol:
		{
			const auto annos = consumeAnnotation();
			annotations.values.insert(annos.begin(), annos.end());
			break;
		}
		case Token::Keyword_Enum:
			file.enums.push_back(consumeEnum(annotations));
			annotations.values.clear();
			break;
		case Token::Keyword_Struct:
			file.structs.push_back(consumeStruct(annotations));
			annotations.values.clear();
			break;
		case Token::EndOfFile:
			return file;
		default:
			ASSERT(false); // should NEVER happen
		}
	}
	return file;
}

Struct Parser::consumeStruct(const Annotations &annotations)
{
	const Token identifier = consumeToken(Token::Identifier);
	consumeToken(Token::CurlyBracketOpen);
	std::vector<Attribute> attributes;

	while (it.hasNext() && it.peekNext().type != Token::CurlyBracketClose)
	{
		Annotations attributeAnnotations;
		while (it.hasNext() && it.peekNext().type == Token::AtSymbol)
		{
			consumeToken(Token::AtSymbol);
			const auto annos = consumeAnnotation();
			attributeAnnotations.values.insert(annos.begin(), annos.end());
		}

		const Token attributeIdentifier = consumeToken(Token::Identifier);
		Type::Ptr attributeType = consumeType();

		Token index;
		if (it.peekNext().type == Token::Equal)
		{
			consumeToken(Token::Equal);
			index = consumeToken(Token::Integer);
		}

		consumeToken(Token::SemiColon);
		attributes.emplace_back(attributeType, index.integer, attributeIdentifier.string, attributeAnnotations);
	}

	consumeToken(Token::CurlyBracketClose);
	return Struct{identifier.string, std::move(attributes), annotations};
}

Enum Parser::consumeEnum(const Annotations &annotations)
{
	const Token identifier = consumeToken(Token::Identifier);
	consumeToken(Token::AngleBracketOpen);
	const Token type = consumeToken(Token::Identifier);
	consumeToken(Token::AngleBracketClose);
	consumeToken(Token::CurlyBracketOpen);

	std::vector<EnumEntry> entries;
	while (it.hasNext() && it.peekNext().type != Token::CurlyBracketClose)
	{
		Annotations entryAnnotations;
		while (it.hasNext() && it.peekNext().type == Token::AtSymbol)
		{
			consumeToken(Token::AtSymbol);
			const auto annos = consumeAnnotation();
			entryAnnotations.values.insert(annos.begin(), annos.end());
		}

		const Token entryIdentifier = consumeToken(Token::Identifier);
		consumeToken(Token::Equal);
		const Token index = consumeToken(Token::Integer);
		consumeToken(Token::SemiColon);
		entries.emplace_back(entryIdentifier.string, index.integer, entryAnnotations);
	}

	consumeToken(Token::CurlyBracketClose);
	return Enum{identifier.string, type.string, entries, annotations};
}

std::unordered_map<std::string, Annotations::Value> Parser::consumeAnnotation()
{
	std::vector<std::string> name = {consumeToken(Token::Identifier).string};
	while (it.peekNext().type == Token::Dot) {
		consumeToken(Token::Dot);
		name.push_back(consumeToken(Token::Identifier).string);
	}
	std::unordered_map<std::string, Annotations::Value> values;
	if (it.peekNext().type == Token::ParanthesisOpen) {
		consumeToken(Token::ParanthesisOpen);
		std::vector<std::string> currentName = name;
		while (true) {
			const Token first = consumeToken({Token::Identifier, Token::String, Token::Integer, Token::ParanthesisClose, Token::Comma});
			if (first.type == Token::ParanthesisClose) {
				break;
			}

			switch (first.type)
			{
			case Token::Identifier:
			{
				while (it.peekNext().type == Token::Dot) {
					consumeToken(Token::Dot);
					currentName.push_back(consumeToken(Token::Identifier).string);
				}
				consumeToken(Token::Equal);
				const Token second = consumeToken({Token::Identifier, Token::String, Token::Integer});
				switch (second.type)
				{
				case Token::Identifier:
				case Token::String:
					values.emplace(StringUtil::joinStrings(currentName, "."), second.string);
					currentName = name;
					break;
				case Token::Integer:
					values.emplace(StringUtil::joinStrings(currentName, "."), second.integer);
					currentName = name;
					break;
				default:
					ASSERT(false); // should NEVER happen
				}
				break;
			}
			case Token::String:
				values.emplace(StringUtil::joinStrings(name, "."), first.string);
				break;
			case Token::Integer:
				values.emplace(StringUtil::joinStrings(name, "."), first.integer);
				break;
			default:
				ASSERT(false); // should NEVER happen
			}

			const Token next = consumeToken({Token::ParanthesisClose, Token::Comma});
			if (next.type == Token::ParanthesisClose) {
				break;
			}
		}
	} else {
		values.emplace(StringUtil::joinStrings(name, "."), std::string());
	}
	return values;
}

Type::Ptr Parser::consumeType()
{
	const Token id = consumeToken(Token::Identifier);
	std::vector<Type::Ptr> args;
	if (it.peekNext().type == Token::AngleBracketOpen) {
		consumeToken(Token::AngleBracketOpen);
		while (true) {
			args.push_back(consumeType());
			const Token next = consumeToken({Token::AngleBracketClose, Token::Comma});
			if (next.type == Token::AngleBracketClose) {
				break;
			}
		}
	}
	return std::make_unique<Type>(id.string, std::move(args));
}

Lexer::Token Parser::consumeToken(const std::initializer_list<Token::Type> &types)
{
	if (!it.hasNext())
	{
		throw UnexpectedEndOfTokenStreamException();
	}
	const Token next = it.next();
	if (std::find(types.begin(), types.end(), next.type) == types.end() &&
			types.size() >= 1 && // if types is empty we allow any type of token
			*(types.begin()) == Token::Invalid)
	{
		throw UnexpectedTokenException(next, types);
	}
	else
	{
		return next;
	}
}

std::string Parser::UnexpectedTokenException::message(const Parser::Token &actual, const std::initializer_list<Parser::Token::Type> &expected)
{
	using Token = Parser::Token;
	const std::vector<Token::Type> tokens(expected);

	std::string expectedString;
	if (tokens.size() == 1)
	{
		expectedString = Token::toString(tokens.front());
	}
	else
	{
		std::vector<std::string> typeStrings;
		std::transform(tokens.begin(), tokens.end(), std::back_inserter(typeStrings), &Token::toString);
		expectedString = StringUtil::joinStrings(typeStrings, ", ");
	}
	return std::string("Got token ") + Token::toString(actual.type) + ", expected " + expectedString;
}

}
}
