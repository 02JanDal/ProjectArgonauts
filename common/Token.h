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

#pragma once

#include <string>
#include <memory>

#include "util/Variant.h"

namespace Argonauts {
namespace Common {
class Token
{
public:
	enum TokenType
	{
		Identifier,
		String,
		Integer,
		Comment,

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
		Equal,
		EndOfFile,

		Invalid,
		Error,

		// specials, used by the parser
		Struct,
		Enum,
		Using,
		Annotation,
		Type,
		TypeList
	};

	explicit Token() : type(Invalid) {}
	template <typename T>
	explicit Token(const TokenType &type_, const T &data_, const int offset_, const int length_ = -1)
		: data(data_), type(type_), offset(offset_), length(length_) {}

	std::string string() const { return data.get<std::string>(); }
	int64_t integer() const { return data.get<int64_t>(); }
	template <typename T> std::shared_ptr<T> getRawData() const
	{
		return std::static_pointer_cast<T>(data.get<std::shared_ptr<void>>());
	}
	template <typename T> void setRawData(T *d)
	{
		data = std::shared_ptr<void>(reinterpret_cast<void *>(d), [](void *p) { delete reinterpret_cast<T *>(p); });
	}
	Util::Variant<std::string, int64_t, std::shared_ptr<void>> data = std::string();

	TokenType type;
	int offset, length;

	static const std::string toString(const TokenType type);

	static Token createIdentifier(const std::string &name, const int offset) { return Token(Identifier, name, offset, int(name.size())); }
	static Token createString(const std::string &string, const int offset, const int length) { return Token(String, string, offset, length); }
	static Token createInteger(const int64_t &integer, const int offset, const int length) { return Token(Integer, integer, offset, length); }
	static Token createSpecial(const TokenType type, const int offset, const int length = 1) { return Token(type, std::string(), offset, length); }
	static Token createComment(const std::string &string, const int offset) { return Token(Comment, string, offset, int(string.size())); }
	static Token createError(const std::string &message, const int offset, const int length) { return Token(Error, message, offset, length); }
};
}
}
