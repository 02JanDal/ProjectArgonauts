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
#include "util/Error.h"
#include "util/StringUtil.h"
#include "common/GeneralParser.h"
#include "DataTypes.h"

namespace Argonauts {
namespace Tool {
using namespace Util;
using namespace Common;

inline PositionedString positionedStringFromToken(const Token &token)
{
	return PositionedString{token.string(), token.offset, token.length};
}
inline PositionedInt64 positionedIntFromToken(const Token &token)
{
	return PositionedInt64{token.integer(), token.offset, token.length};
}

Parser::Parser(const std::vector<Token> &tokens)
	: m_tokens(tokens)
{
}

File Parser::process()
{
	File file;
	Annotations annotations;
	PositionedString lastDefinedAnnotation;

	auto defineStruct = [&file, &annotations](const Token &token, const std::vector<Token> &stack)
	{
		file.structs.push_back(Struct{PositionedString(), positionedStringFromToken(stack.at(stack.size() - 1)), {}, annotations});
		annotations.values.clear();
		return GeneralParser::defineStruct(token, stack);
	};
	auto defineStructWithInclude = [&file, &annotations](const Token &token, const std::vector<Token> &stack)
	{
		file.structs.push_back(Struct{positionedStringFromToken(stack.at(stack.size() - 1)), positionedStringFromToken(stack.at(stack.size() - 3)), {}, annotations});
		annotations.values.clear();
		return GeneralParser::defineStructWithInclude(token, stack);
	};
	auto defineEnum = [&file, &annotations](const Token &token, const std::vector<Token> &stack)
	{
		file.enums.push_back(Enum{positionedStringFromToken(stack.at(stack.size() - 4)), positionedStringFromToken(stack.at(stack.size() - 2)), {}, annotations});
		annotations.values.clear();
		return GeneralParser::defineEnum(token, stack);
	};
	auto defineUsing = [&file, &annotations](const Token &token, const std::vector<Token> &stack)
	{
		file.usings.push_back(Using{positionedStringFromToken(stack.at(stack.size() - 3)), stack.at(stack.size() - 1).getRawData<Type>(), annotations});
		annotations.values.clear();
		return GeneralParser::defineUsing(token, stack);
	};
	auto defineAnnotation = [&annotations, &lastDefinedAnnotation](const Token &token, const std::vector<Token> &stack)
	{
		annotations.values.insert({positionedStringFromToken(token), Annotations::Value(PositionedString())});
		lastDefinedAnnotation = token.string();
		return GeneralParser::defineAnnotation(token, stack);
	};
	auto addToAnnotation = [&file, &annotations, &lastDefinedAnnotation](const Token &token, const std::vector<Token> &stack)
	{
		PositionedString name;
		Annotations::Value value = PositionedString();
		if (stack.at(stack.size() - 2).type == Token::Equal) {
			name = positionedStringFromToken(stack.at(stack.size() - 3));
			name = PositionedString(lastDefinedAnnotation.value + '.' + name.value, name.offset, name.length);
			const Token valueToken = stack.back();
			if (valueToken.type == Token::Integer) {
				value = positionedIntFromToken(valueToken);
			} else {
				value = positionedStringFromToken(valueToken);
			}
		} else {
			name = lastDefinedAnnotation;
			if (stack.back().type == Token::Integer) {
				value = positionedIntFromToken(stack.back());
			} else {
				value = positionedStringFromToken(stack.back());
			}
		}

		annotations.values.erase(lastDefinedAnnotation);
		annotations.values.insert({name, value});
		return GeneralParser::addToAnnotation(token, stack);
	};
	auto defineEnumEntry = [&file, &annotations](const Token &token, const std::vector<Token> &stack)
	{
		const PositionedString name = positionedStringFromToken(stack.at(stack.size() - 3));
		const PositionedInt64 index = positionedIntFromToken(stack.at(stack.size() - 1));
		file.enums.back().entries.push_back(EnumEntry{name, index, annotations});
		annotations.values.clear();
		return GeneralParser::defineEnumEntry(token, stack);
	};
	auto defineStructAttribute = [&file, &annotations](const Token &token, const std::vector<Token> &stack)
	{
		const PositionedString name = positionedStringFromToken(stack.at(stack.size() - 2));
		Type::Ptr type = stack.at(stack.size() - 1).getRawData<Type>();
		file.structs.back().members.push_back(Attribute{type, PositionedInt64(), name, annotations});
		annotations.values.clear();
		return GeneralParser::defineStructAttribute(token, stack);
	};
	auto defineStructAttributeWithIndex = [&file, &annotations](const Token &token, const std::vector<Token> &stack)
	{
		const PositionedString name = positionedStringFromToken(stack.at(stack.size() - 4));
		const Type::Ptr type = stack.at(stack.size() - 3).getRawData<Type>();
		const PositionedInt64 index = positionedIntFromToken(stack.at(stack.size() - 1));
		file.structs.back().members.push_back(Attribute{type, index, name, annotations});
		annotations.values.clear();
		return GeneralParser::defineStructAttributeWithIndex(token, stack);
	};
	auto convertTypeToTypeList = [](const Token &token, const std::vector<Token> &stack)
	{
		const Token typeToken = stack.back();
		Type::Ptr type = typeToken.getRawData<Type>();
		Token newTkn;
		newTkn.type = Token::TypeList;
		newTkn.data = stack.at(stack.size() - 3).data;
		newTkn.getRawData<Type>()->templateArguments.push_back(type);
		return std::make_pair(1, std::vector<Token>({newTkn}));
	};
	auto mergeTypeListAndType = [](const Token &token, const std::vector<Token> &stack)
	{
		Token typelistToken = stack.at(stack.size() - 4);
		Type::Ptr typelistContainer = typelistToken.getRawData<Type>();
		Type::Ptr type = stack.at(stack.size() - 1).getRawData<Type>();
		typelistContainer->templateArguments.push_back(type);
		return std::make_pair(1, std::vector<Token>());
	};
	auto finalizeType = [](const Token &token, const std::vector<Token> &stack)
	{
		if (stack.back().type == Token::TypeList) {
			return std::make_pair(2, std::vector<Token>());
		} else if (stack.at(stack.size() - 2).type == Token::TypeList) {
			Type::Ptr type = stack.at(stack.size() - 1).getRawData<Type>();
			stack.at(stack.size() - 4).getRawData<Type>()->templateArguments.push_back(type);
			return std::make_pair(3, std::vector<Token>());
		} else {
			Type::Ptr type = stack.at(stack.size() - 1).getRawData<Type>();
			stack.at(stack.size() - 3).getRawData<Type>()->templateArguments.push_back(type);
			return std::make_pair(2, std::vector<Token>());
		}
	};
	auto typeDefinition = [&file](const std::vector<Token> &stack)
	{
		const Token id = stack.back();
		if (Type::create(id.string())->isBuiltin()) {
			throw ParserException(std::string("Redefinition of built-in type '") + id.string() + "'", id.offset, id.length);
		}
		for (const std::string &s : file.definedTypes()) {
			if (s == id.string()) {
				throw ParserException(std::string("Redefinition of user-defined type '") + id.string() + "'", id.offset, id.length);
			}
		}
	};
	auto typeUsage = [&file](const std::vector<Token> &stack)
	{
		const Token id = stack.back();
		if (Type::create(id.string())->isBuiltin()) {
			return;
		}
		for (const std::string &s : file.definedTypes()) {
			if (s == id.string()) {
				return;
			}
		}
		throw ParserException(std::string("Unknown type: ") + id.string(), id.offset, id.length);
	};
	auto morphIdentifierToType = [](const Token &token)
	{
		Token tkn;
		tkn.type = Token::Type;
		tkn.offset = token.offset;
		tkn.length = token.length;
		tkn.setRawData(new Type(positionedStringFromToken(token), {}));
		return tkn;
	};
	auto morphTypeToTypeList = [](const Token &token)
	{
		Token tkn = token;
		tkn.type = Token::TypeList;
		return tkn;
	};
	auto unexpected = [](const Token &token)
	{
		switch (token.type) {
		case Token::Identifier: throw ParserException(std::string("Unexpected token IDENTIFIER (") + token.string() + ")", token.offset, token.length);
		case Token::String: throw ParserException(std::string("Unexpected token STRING (\"") + token.string() + "\")", token.offset, token.length);
		case Token::Integer: throw ParserException(std::string("Unexpected token INTEGER (") + std::to_string(token.integer()) + ")", token.offset, token.length);
		default: throw ParserException(std::string("Unexpected token ") + Token::toString(token.type), token.offset, token.length);
		}
	};

	GeneralParser::parse({defineStruct, defineStructWithInclude, defineEnum, defineUsing, defineAnnotation, addToAnnotation,
						  defineEnumEntry, defineStructAttribute, defineStructAttributeWithIndex, convertTypeToTypeList, mergeTypeListAndType, finalizeType},
						 {typeDefinition, typeUsage},
						 {morphIdentifierToType, morphTypeToTypeList},
						 std::vector<Token>(),
						 m_tokens,
						 unexpected);

	return file;
}

std::string Parser::UnexpectedTokenException::message(const Parser::Token &actual, const std::initializer_list<Parser::Token::TokenType> &expected)
{
	using Token = Parser::Token;
	const std::vector<Token::TokenType> tokens(expected);

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
