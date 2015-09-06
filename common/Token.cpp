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

#include "Token.h"

namespace Argonauts {
namespace Common {
const std::string Token::toString(const TokenType type)
{
	switch (type)
	{
	case Identifier: return "IDENTIFIER";
	case String: return "STRING";
	case Integer: return "INTEGER";
	case Comment: return "COMMENT";
	case Keyword_Enum: return "ENUM";
	case Keyword_Struct: return "STRUCT";
	case Keyword_Using: return "USING";
	case ExclamationMark: return "EXCLAMATION_MARK";
	case AtSymbol: return "AT_SYMBOL";
	case ParanthesisOpen: return "PARANTHESIS_OPEN";
	case ParanthesisClose: return "PRARANTHESIS_CLOSE";
	case CurlyBracketOpen: return "CURLY_BRACKET_OPEN";
	case CurlyBracketClose: return "CURLY_BRACKET_CLOSE";
	case AngleBracketOpen: return "ANGLE_BRACKET_OPEN";
	case AngleBracketClose: return "ANGLE_BRACKET_CLOSE";
	case SemiColon: return "SEMICOLON";
	case Comma: return "COMMA";
	case Colon: return "COLON";
	case Equal: return "EQUAL";
	case EndOfFile: return "EOF";
	case Invalid: return "INVALID";
	case Error: return "ERROR";
	case Struct: return "struct";
	case Enum: return "enum";
	case Using: return "using";
	case Annotation: return "annotation";
	case Type: return "type";
	case TypeList: return "typelist";
	}
}
}
}
