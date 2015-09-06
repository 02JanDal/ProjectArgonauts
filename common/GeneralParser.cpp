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

#include "GeneralParser.h"

#include <iostream>

#include "Lexer.h"
#include "Token.h"

#if 0
static std::string joinedTypes(const std::vector<Argonauts::Common::Token> &tokens)
{
	std::string out;
	for (const auto &token : tokens) {
		out += Argonauts::Common::Token::toString(token.type) + ' ';
	}
	return out;
}
# define PARSERLOG if (true)  std::cout
#else
static std::string joinedTypes(const std::vector<Argonauts::Common::Token> &) { return std::string(); }
# define PARSERLOG if (false) std::cout
#endif

namespace Argonauts {
namespace Common {
/* Given a token and a stack of tokens, what action should be performed:
 * <Token>			<Stack>											<Action><ActionData>
 * String			String											Merge	String + String -> String
 *
 * KeywordStruct	(empty)											Shift
 * Identifier		KeywordStruct									Shift	typeDefinition
 * Colon			KeywordStruct,Identifier						Shift
 * Identifier		KeywordStruct,Identifier,Colon					Shift	typeUsage
 * CurlyOpen		KeywordStruct,Identifier						Reduce	defineStruct -> Struct
 * CurlyOpen		KeywordStruct,Identifier,Colon,Identifier		Reduce	defineStructWithInclude -> Struct
 * CurlyClose		Struct											Unshift
 *
 * KeywordEnum		(empty)											Shift
 * Identifier		KeywordEnum										Shift	typeDefinition
 * AngleOpen		KeywordEnum,Identifier							Shift
 * Identifier		KeywordEnum,Identifier,AngleOpen				Shift	typeUsage
 * AngleClose		KeywordEnum,Identifier,AngleOpen,Identifier		Shift
 * CurlyOpen		KeywordEnum,Identifier,AngleOpen,Identifier,AngleClose	Reduce	defineEnum -> Enum
 * CurlyClose		Enum											Unshift
 *
 * KeywordUsing		(empty)											Shift
 * Identifier		KeywordUsing									Shift	typeDefinition
 * Equal			KeywordUsing,Identifier							Shift
 * Identifier		KeywordUsing,Identifier,Equal					Shift typeUsage, Morph	Identifier -> Type
 * Semicolon		KeywordUsing,Identifier,Equal,Type				Reduce	defineUsing, Unshift
 *
 * At				(empty)											Shift
 * Identifier		At												Reduce	defineAnnotation -> Annotation
 * ParanOpen		Annotation										Shift
 * Identifier		Annotation,ParanOpen							Shift
 * String			Annotation,ParanOpen							Shift
 * Integer			Annotation,ParanOpen							Shift
 * ParanClose		Annotation,ParanOpen,Identifier					Reduce	addToAnnotation -> Annotation,ParanOpen, Unshift
 * ParanClose		Annotation,ParanOpen,String						Reduce	addToAnnotation -> Annotation,ParanOpen, Unshift
 * ParanClose		Annotation,ParanOpen,Integer					Reduce	addToAnnotation -> Annotation,ParanOpen, Unshift
 * Equal			Annotation,ParanOpen,Identifier					Shift
 * Identifier		Annotation,ParanOpen,Identifier,Equal			Shift
 * String			Annotation,ParanOpen,Identifier,Equal			Shift
 * Integer			Annotation,ParanOpen,Identifier,Equal			Shift
 * ParanClose		Annotation,ParanOpen,Identifier,Equal,Identifier	Reduce addToAnnotation -> Annotation,ParanOpen, Unshift
 * ParanClose		Annotation,ParanOpen,Identifier,Equal,String	Reduce	addToAnnotation -> Annotation,ParanOpen, Unshift
 * ParanClose		Annotation,ParanOpen,Identifier,Equal,Integer	Reduce	addToAnnotation -> Annotation,ParanOpen, Unshift
 * Comma			Annotation,ParanOpen,Identifier					Reduce	addToAnnotation -> Annotation,ParanOpen
 * Comma			Annotation,ParanOpen,String						Reduce	addToAnnotation -> Annotation,ParanOpen
 * Comma			Annotation,ParanOpen,Integer					Reduce	addToAnnotation -> Annotation,ParanOpen
 * Comma			Annotation,ParanOpen,Identifier,Equal,Identifier	Reduce	addToAnnotation -> Annotation,ParanOpen
 * Comma			Annotation,ParanOpen,Identifier,Equal,String	Reduce	addToAnnotation -> Annotation,ParanOpen
 * Comma			Annotation,ParanOpen,Identifier,Equal,Integer	Reduce	addToAnnotation -> Annotation,ParanOpen
 *
 * Identifier		Enum											Shift	typeDefinition
 * Equal			Enum,Identifier									Shift
 * Integer			Enum,Identifier,Equal							Shift
 * Semicolon		Enum,Identifier,Equal,Integer					Reduce	defineEnumEntry -> Enum
 *
 * Identifier		Struct											Shift	typeDefinition
 * Identifier		Struct,Identifier								Shift	typeUsage, Morph Identifier -> Type
 * Semicolon		Struct,Identifier,Type							Reduce	defineStructAttribute -> Struct
 * Equal			Struct,Identifier,Type							Shift
 * Integer			Struct,Identifier,Type,Equal					Shift
 * Semicolon		Struct,Identifier,Type,Equal,Integer			Reduce	defineStructAttributeWithIndex -> Struct
 *
 * AngleOpen		Type											Shift
 * Identifier		Type,AngleOpen									Shift typeUsage, Morph	Identifier -> Type
 * Comma			Type,AngleOpen,Type								Reduce	convertTypeToTypeList Type -> TypeList
 * Identifier		Type,AngleOpen,TypeList							Shift typeUsage, Morph	Identifier -> Type
 * Comma			Type,AngleOpen,TypeList,Type					Reduce	mergeTypeListAndType TypeList,Type -> TypeList
 * AngleClose		Type,AngleOpen,Type								Reduce	finalizeType -> Type
 * AngleClose		Type,AngleOpen,TypeList							Reduce	finalizeType -> Type
 * AngleClose		Type,AngleOpen,TypeList,Type					Reduce	finalizeType -> Type
 */
std::vector<Token> GeneralParser::parse(const std::vector<GeneralParser::ReduceFunction> &reducers, const std::vector<ShiftFunction> &shifters, const std::vector<GeneralParser::MorphFunction> &morphers, const std::vector<Token> &startStack, const std::vector<Token> tokens, UnexpectedFunction unexpected)
{
	enum
	{
		defineStruct = 0,
		defineStructWithInclude = 1,
		defineEnum = 2,
		defineUsing = 3,
		defineAnnotation = 4,
		addToAnnotation = 5,
		defineEnumEntry = 6,
		defineStructAttribute = 7,
		defineStructAttributeWithIndex = 8,
		convertTypeToTypeList = 9,
		mergeTypeListAndType = 10,
		finalizeType = 11,

		typeDefinition = 0,
		typeUsage = 1,

		morphIdentifierToType = 0,
		morphTypeToTypeList = 1
	};

	std::vector<Token> stack = startStack;

	// check helper
	auto checkStack = [&stack](const std::initializer_list<Token::TokenType> &types)
	{
		if (stack.size() < types.size()) {
			return false;
		}
		int i = stack.size() - types.size();
		auto it = types.begin();
		for (; it != types.end(); ++i, ++it) {
			if (stack.at(i).type != *it) {
				return false;
			}
		}
		return true;
	};

	// actions
	auto shift = [&stack, shifters](const Token &token, const int function = -1)
	{
		PARSERLOG << "> SHIFT " << Token::toString(token.type) << "\n";
		stack.push_back(token);
		PARSERLOG << "\t" << joinedTypes(stack) << "\n";
		if (function != -1) {
			shifters[function](stack);
		}
	};
	auto reduce = [&stack, reducers](const Token &token, const int function)
	{
		PARSERLOG << "- REDUCE " << joinedTypes(stack) << "+ " << Token::toString(token.type) << "\n";
		const auto result = reducers[function](token, stack);
		for (int i = 0; i < result.first; ++i) {
			stack.pop_back();
		}
		std::copy(result.second.begin(), result.second.end(), std::back_inserter(stack));
		PARSERLOG << "\t" << joinedTypes(stack) << "\n";
	};
	auto unshift = [&stack]() { PARSERLOG << "< UNSHIFT\n"; stack.pop_back(); PARSERLOG << "\t" << joinedTypes(stack) << "\n"; };
	auto merge = [&stack](const Token &token)
	{
		PARSERLOG << "+ MERGE " << Token::toString(stack.back().type) << " + " << Token::toString(token.type) << "\n";
		if (stack.empty()) {
			return;
		}
		if (stack.back().type == Token::String && token.type == Token::String) {
			stack.back().data = stack.back().string() + token.string();
			stack.back().length += token.length;
		}
		PARSERLOG << "\t" << joinedTypes(stack) << "\n";
	};
	auto morph = [&stack, morphers](const int function)
	{
		PARSERLOG << "! MORPH " << Token::toString(stack.back().type) << "\n";
		if (stack.empty()) {
			return;
		}
		stack.back() = morphers[function](stack.back());
		PARSERLOG << "\t" << joinedTypes(stack) << "\n";
	};

	for (const Token &token : tokens) {
		PARSERLOG << "################## READ " << Token::toString(token.type) << " ##################\n";
		switch (token.type) {
		case Token::Identifier: {
			if (checkStack({Token::Keyword_Struct})) {
				shift(token, typeDefinition);
			} else if (checkStack({Token::Keyword_Struct, Token::Identifier, Token::Colon})) {
				shift(token, typeUsage);
			} else if (checkStack({Token::Keyword_Enum})) {
				shift(token, typeDefinition);
			} else if (checkStack({Token::Keyword_Enum, Token::Identifier, Token::AngleBracketOpen})) {
				shift(token, typeUsage);
			} else if (checkStack({Token::Keyword_Using})) {
				shift(token, typeDefinition);
			} else if (checkStack({Token::Keyword_Using, Token::Identifier, Token::Equal})) {
				shift(token, typeUsage);
				morph(morphIdentifierToType);
			} else if (checkStack({Token::AtSymbol})) {
				reduce(token, defineAnnotation);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen})) {
				shift(token);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal})) {
				shift(token);
			} else if (checkStack({Token::Enum})) {
				shift(token, typeDefinition);
			} else if (checkStack({Token::Enum, Token::Annotation})) {
				unshift();
				shift(token, typeDefinition);
			} else if (checkStack({Token::Struct})) {
				shift(token, typeDefinition);
			} else if (checkStack({Token::Struct, Token::Annotation})) {
				unshift();
				shift(token, typeDefinition);
			} else if (checkStack({Token::Struct, Token::Identifier})) {
				shift(token, typeUsage);
				morph(morphIdentifierToType);
			} else if (checkStack({Token::Type, Token::AngleBracketOpen})) {
				shift(token, typeUsage);
				morph(morphIdentifierToType);
			} else if (checkStack({Token::Type, Token::AngleBracketOpen, Token::TypeList})) {
				shift(token, typeUsage);
				morph(morphIdentifierToType);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::String: {
			if (checkStack({Token::String})) {
				merge(token);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen})) {
				shift(token);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal})) {
				shift(token);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::Integer: {
			if (checkStack({Token::Annotation, Token::ParanthesisOpen})) {
				shift(token);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal})) {
				shift(token);
			} else if (checkStack({Token::Enum, Token::Identifier, Token::Equal})) {
				shift(token);
			} else if (checkStack({Token::Struct, Token::Identifier, Token::Type, Token::Equal})) {
				shift(token);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::Comment: break;

		case Token::Keyword_Enum:
		case Token::Keyword_Struct:
		case Token::Keyword_Using: {
			if (stack.empty()) {
				shift(token);
			} else if (stack.back().type == Token::Annotation) {
				unshift();
				shift(token);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::ExclamationMark: unexpected(token); break;
		case Token::AtSymbol: {
			if (stack.empty()) {
				shift(token);
			} else if (stack.back().type == Token::Annotation) {
				unshift();
				shift(token);
			} else if (stack.back().type == Token::Struct || stack.back().type == Token::Enum) {
				shift(token);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::ParanthesisOpen: {
			if (checkStack({Token::Annotation})) {
				shift(token);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::ParanthesisClose: {
			if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier})) {
				reduce(token, addToAnnotation);
				unshift();
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::String})) {
				reduce(token, addToAnnotation);
				unshift();
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Integer})) {
				reduce(token, addToAnnotation);
				unshift();
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal, Token::Identifier})) {
				reduce(token, addToAnnotation);
				unshift();
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal, Token::String})) {
				reduce(token, addToAnnotation);
				unshift();
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal, Token::Integer})) {
				reduce(token, addToAnnotation);
				unshift();
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::CurlyBracketOpen: {
			if (checkStack({Token::Keyword_Struct, Token::Identifier})) {
				reduce(token, defineStruct);
			} else if (checkStack({Token::Keyword_Struct, Token::Identifier, Token::Colon, Token::Identifier})) {
				reduce(token, defineStructWithInclude);
			} else if (checkStack({Token::Keyword_Enum, Token::Identifier, Token::AngleBracketOpen, Token::Identifier, Token::AngleBracketClose})) {
				reduce(token, defineEnum);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::CurlyBracketClose: {
			if (stack.back().type == Token::Struct || stack.back().type == Token::Enum) {
				unshift();
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::AngleBracketOpen: {
			if (checkStack({Token::Keyword_Enum, Token::Identifier})) {
				shift(token);
			} else if (checkStack({Token::Type})) {
				shift(token);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::AngleBracketClose: {
			if (checkStack({Token::Keyword_Enum, Token::Identifier, Token::AngleBracketOpen, Token::Identifier})) {
				shift(token);
			} else if (checkStack({Token::Type, Token::AngleBracketOpen, Token::Type})) {
				reduce(token, finalizeType);
			} else if (checkStack({Token::Type, Token::AngleBracketOpen, Token::TypeList})) {
				reduce(token, finalizeType);
			} else if (checkStack({Token::Type, Token::AngleBracketOpen, Token::TypeList, Token::Type})) {
				reduce(token, finalizeType);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::SemiColon: {
			if (checkStack({Token::Keyword_Using, Token::Identifier, Token::Equal, Token::Type})) {
				reduce(token, defineUsing);
				unshift();
			} else if (checkStack({Token::Enum, Token::Identifier, Token::Equal, Token::Integer})) {
				reduce(token, defineEnumEntry);
			} else if (checkStack({Token::Struct, Token::Identifier, Token::Type})) {
				reduce(token, defineStructAttribute);
			} else if (checkStack({Token::Struct, Token::Identifier, Token::Type, Token::Equal, Token::Integer})) {
				reduce(token, defineStructAttributeWithIndex);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::Colon: {
			if (checkStack({Token::Keyword_Struct, Token::Identifier})) {
				shift(token);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::Comma: {
			if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier})) {
				reduce(token, addToAnnotation);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::String})) {
				reduce(token, addToAnnotation);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Integer})) {
				reduce(token, addToAnnotation);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal, Token::Identifier})) {
				reduce(token, addToAnnotation);
			} else  if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal, Token::String})) {
				reduce(token, addToAnnotation);
			} else  if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier, Token::Equal, Token::Integer})) {
				reduce(token, addToAnnotation);
			} else if (checkStack({Token::Type, Token::AngleBracketOpen, Token::Type})) {
				reduce(token, convertTypeToTypeList);
			} else if (checkStack({Token::Type, Token::AngleBracketOpen, Token::TypeList, Token::Type})) {
				reduce(token, mergeTypeListAndType);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::Equal: {
			if (checkStack({Token::Keyword_Using, Token::Identifier})) {
				shift(token);
			} else if (checkStack({Token::Annotation, Token::ParanthesisOpen, Token::Identifier})) {
				shift(token);
			} else if (checkStack({Token::Enum, Token::Identifier})) {
				shift(token);
			} else if (checkStack({Token::Struct, Token::Identifier, Token::Type})) {
				shift(token);
			} else {
				unexpected(token);
			}
			break;
		}
		case Token::EndOfFile: {
			if (!stack.empty()) {
				unexpected(token);
			}
			break;
		}
		case Token::Invalid: break;
		case Token::Error:
			unexpected(token);
			break;

		case Token::Struct: break;
		case Token::Enum: break;
		case Token::Annotation: break;
		}
	}
	return stack;
}

GeneralParser::ReduceCommand GeneralParser::defineStruct(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(2, std::vector<Token>({Token::createSpecial(Token::Struct, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::defineStructWithInclude(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(4, std::vector<Token>({Token::createSpecial(Token::Struct, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::defineEnum(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(5, std::vector<Token>({Token::createSpecial(Token::Enum, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::defineUsing(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(4, std::vector<Token>({Token::createSpecial(Token::Using, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::defineAnnotation(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(1, std::vector<Token>({Token::createSpecial(Token::Annotation, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::addToAnnotation(const Token &token, const std::vector<Token> &stack)
{
	for (int i = (stack.size() - 1); i >= 0; --i) {
		if (stack.at(i).type == Token::ParanthesisOpen) {
			return std::make_pair(stack.size() - i, std::vector<Token>({Token::createSpecial(Token::ParanthesisOpen, -1)}));
		}
	}
	return std::make_pair(0, std::vector<Token>());
}
GeneralParser::ReduceCommand GeneralParser::defineEnumEntry(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(4, std::vector<Token>({Token::createSpecial(Token::Enum, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::defineStructAttribute(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(3, std::vector<Token>({Token::createSpecial(Token::Struct, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::defineStructAttributeWithIndex(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(5, std::vector<Token>({Token::createSpecial(Token::Struct, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::convertTypeToTypeList(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(1, std::vector<Token>({Token::createSpecial(Token::TypeList, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::mergeTypeListAndType(const Token &token, const std::vector<Token> &stack)
{
	return std::make_pair(2, std::vector<Token>({Token::createSpecial(Token::TypeList, -1)}));
}
GeneralParser::ReduceCommand GeneralParser::finalizeType(const Token &token, const std::vector<Token> &stack)
{
	if (stack.back().type == Token::TypeList) {
		return std::make_pair(3, std::vector<Token>({Token::createSpecial(Token::Type, -1)}));
	} else if (stack.at(stack.size() - 2).type == Token::TypeList) {
		return std::make_pair(4, std::vector<Token>({Token::createSpecial(Token::Type, -1)}));
	} else {
		return std::make_pair(3, std::vector<Token>({Token::createSpecial(Token::Type, -1)}));
	}
}
}
}
