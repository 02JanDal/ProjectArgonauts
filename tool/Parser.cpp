#include "Parser.h"

#include <algorithm>

#include "util/Util.h"
#include "util/StringUtil.h"
#include "DataTypes.h"

using namespace Argonauts;

Parser::Parser(const std::vector<Lexer::Token> &tokens)
	: it(tokens)
{
}

Argonauts::File Parser::process()
{
	File file;
	std::vector<Annotation> annotations;
	while (it.hasNext())
	{
		const Token next = consumeToken({Token::AtSymbol, Token::Keyword_Enum, Token::Keyword_Struct, Token::EndOfFile});
		switch (next.type)
		{
		case Token::AtSymbol:
			annotations.push_back(consumeAnnotation());
			break;
		case Token::Keyword_Enum:
			file.enums.push_back(consumeEnum(annotations));
			annotations.clear();
			break;
		case Token::Keyword_Struct:
			file.structs.push_back(consumeStruct(annotations));
			annotations.clear();
			break;
		case Token::EndOfFile:
			return file;
		default:
			ASSERT(false); // should NEVER happen
		}
	}
	return file;
}

Struct Parser::consumeStruct(const std::vector<Annotation> &annotations)
{
	const Token identifier = consumeToken(Token::Identifier);
	consumeToken(Token::CurlyBracketOpen);
	std::vector<Attribute> attributes;

	while (it.hasNext() && it.peekNext().type != Token::CurlyBracketClose)
	{
		std::vector<Annotation> annotations;
		while (it.hasNext() && it.peekNext().type == Token::AtSymbol)
		{
			consumeToken(Token::AtSymbol);
			annotations.push_back(consumeAnnotation());
		}

		const Token attributeIdentifier = consumeToken(Token::Identifier);
		const Token attributeType = consumeToken(Token::Identifier);

		Token templateType;
		if (it.peekNext().type == Token::AngleBracketOpen)
		{
			consumeToken(Token::AngleBracketOpen);
			templateType = consumeToken(Token::Identifier);
			consumeToken(Token::AngleBracketClose);
		}

		Token index;
		if (it.peekNext().type == Token::Equal)
		{
			consumeToken(Token::Equal);
			index = consumeToken(Token::Integer);
		}

		consumeToken(Token::SemiColon);
		attributes.emplace_back(attributeType.string, templateType.string, index.integer, attributeIdentifier.string, annotations);
	}

	consumeToken(Token::CurlyBracketClose);
	return Struct{identifier.string, attributes, annotations};
}

Enum Parser::consumeEnum(const std::vector<Annotation> &annotations)
{
	const Token identifier = consumeToken(Token::Identifier);
	consumeToken(Token::AngleBracketOpen);
	const Token type = consumeToken(Token::Identifier);
	consumeToken(Token::AngleBracketClose);
	consumeToken(Token::CurlyBracketOpen);

	std::vector<EnumEntry> entries;
	while (it.hasNext() && it.peekNext().type != Token::CurlyBracketClose)
	{
		std::vector<Annotation> annotations;
		while (it.hasNext() && it.peekNext().type == Token::AtSymbol)
		{
			consumeToken(Token::AtSymbol);
			annotations.push_back(consumeAnnotation());
		}

		const Token entryIdentifier = consumeToken(Token::Identifier);
		consumeToken(Token::Equal);
		const Token index = consumeToken(Token::Integer);
		consumeToken(Token::SemiColon);
		entries.emplace_back(entryIdentifier.string, index.integer, annotations);
	}

	consumeToken(Token::CurlyBracketClose);
	return Enum{identifier.string, type.string, entries, annotations};
}

Annotation Parser::consumeAnnotation()
{
	const Token identifier = consumeToken(Token::Identifier);
	std::vector<AnnotationArgument> arguments;
	if (it.peekNext().type == Token::ParanthesisOpen)
	{
		consumeToken(Token::ParanthesisOpen);
		while (it.hasNext() && it.peekNext().type != Token::ParanthesisClose)
		{
			AnnotationArgument argument;
			const Token first = consumeToken({Token::Identifier, Token::String, Token::Integer});
			switch (first.type)
			{
			case Token::Identifier:
			{
				if (it.peekNext().type == Token::Equal)
				{
					consumeToken(Token::Equal);
					const Token second = consumeToken({Token::Identifier, Token::String, Token::Integer});
					argument.key = first.string;
					switch (second.type)
					{
					case Token::Identifier:
						argument.valueString = second.string;
						argument.valueType = AnnotationArgument::Identifier;
						break;
					case Token::String:
						argument.valueString = second.string;
						argument.valueType = AnnotationArgument::String;
						break;
					case Token::Integer:
						argument.valueInteger = second.integer;
						argument.valueType = AnnotationArgument::Integer;
					default:
						ASSERT(false); // should NEVER happen
					}
				}
				else
				{
					argument.valueString = first.string;
					argument.valueType = AnnotationArgument::Identifier;
				}
				break;
			}
			case Token::String:
				argument.valueString = first.string;
				argument.valueType = AnnotationArgument::String;
				break;
			case Token::Integer:
				argument.valueInteger = first.integer;
				argument.valueType = AnnotationArgument::Integer;
			default:
				ASSERT(false); // should NEVER happen
			}
			arguments.push_back(argument);
		}
		consumeToken(Token::ParanthesisClose);
	}
	return Annotation{identifier.string, arguments};
}

Lexer::Token Parser::consumeToken(const std::initializer_list<Token::Type> &types)
{
	if (!it.hasNext())
	{
		throw UnexpectedEndOfTokenStreamException();
	}
	const Token next = it.next();
	if (std::find(types.begin(), types.end(), next.type) == types.end() && types.size() == 1 && *(types.begin()) == Token::Invalid)
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
