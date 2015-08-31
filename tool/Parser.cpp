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

#include "Parser.h"

#include <algorithm>
#include <iostream>

#include "util/Util.h"
#include "util/StringUtil.h"
#include "DataTypes.h"

namespace Argonauts {
using namespace Util;
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
		const Token next = consumeToken({Token::AtSymbol, Token::Keyword_Enum, Token::Keyword_Struct, Token::Keyword_Using, Token::EndOfFile});
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
		case Token::Keyword_Using: {
			const Token name = consumeToken(Token::Identifier);
			consumeToken(Token::Equal);
			file.usings.push_back(Using{name.string, consumeType(), annotations});
			consumeToken(Token::SemiColon);
			annotations.values.clear();
			break;
		}
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
	const Token next = consumeToken({Token::CurlyBracketOpen, Token::Colon});
	Token includes;
	if (next.type == Token::Colon) {
		includes = consumeToken(Token::Identifier);
		consumeToken(Token::CurlyBracketOpen);
	}

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

		const PositionedString attributeName = consumeAttributeName();
		Type::Ptr attributeType = consumeType();

		Token index;
		if (it.peekNext().type == Token::Equal)
		{
			consumeToken(Token::Equal);
			index = consumeToken(Token::Integer);
		}

		consumeToken(Token::SemiColon);
		attributes.emplace_back(attributeType, PositionedInt64(index.integer, index.offset), attributeName, attributeAnnotations);
	}

	consumeToken(Token::CurlyBracketClose);
	return Struct{PositionedString(includes.string, includes.offset), PositionedString(identifier.string, identifier.offset),
				std::move(attributes), annotations};
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
		entries.emplace_back(PositionedString(entryIdentifier.string, entryIdentifier.offset), PositionedInt64(index.integer, index.offset), entryAnnotations);
	}

	consumeToken(Token::CurlyBracketClose);
	return Enum{PositionedString(identifier.string, identifier.offset), PositionedString(type.string, type.offset), entries, annotations};
}

std::unordered_multimap<PositionedString, Annotations::Value> Parser::consumeAnnotation()
{
	const Token initialToken = consumeToken(Token::Identifier);
	std::vector<std::string> name = {initialToken.string};
	while (it.peekNext().type == Token::Dot) {
		consumeToken(Token::Dot);
		name.push_back(consumeToken(Token::Identifier).string);
	}
	std::unordered_multimap<PositionedString, Annotations::Value> values;
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
				currentName.push_back(first.string);
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
					values.emplace(PositionedString(String::joinStrings(currentName, "."), first.offset), Annotations::Value(second.string));
					currentName = name;
					break;
				case Token::Integer:
					values.emplace(PositionedString(String::joinStrings(currentName, "."), first.offset), Annotations::Value(second.integer));
					currentName = name;
					break;
				default:
					ASSERT(false); // should NEVER happen
				}
				break;
			}
			case Token::String:
				values.emplace(PositionedString(String::joinStrings(name, "."), initialToken.offset), Annotations::Value(first.string));
				break;
			case Token::Integer:
				values.emplace(PositionedString(String::joinStrings(name, "."), initialToken.offset), Annotations::Value(first.integer));
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
		values.emplace(PositionedString(String::joinStrings(name, "."), initialToken.offset), Annotations::Value(std::string()));
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
	return std::make_unique<Type>(PositionedString(id.string, id.offset), std::move(args));
}

PositionedString Parser::consumeAttributeName()
{
	const Token start = consumeToken(Token::Identifier);
	std::string name = start.string;
	while (it.peekNext().type == Token::Dot) {
		consumeToken(Token::Dot);
		name += '.' + consumeToken(Token::Identifier).string;
	}
	return PositionedString(name, start.offset);
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
			*(types.begin()) != Token::Invalid)
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
		expectedString = String::joinStrings(typeStrings, ", ");
	}
	return std::string("Got token ") + Token::toString(actual.type) + ", expected " + expectedString;
}

}
}
