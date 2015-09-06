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
#include <vector>
#include <functional>

#include "util/SelfContainerIterator.h"

namespace Argonauts {
namespace Common {
class Token;

class Lexer
{
public:
	explicit Lexer();

	enum StartState
	{
		Normal,
		InString
	};
	void setState(const StartState state) { m_state = state; }
	StartState state() const { return m_state; }

	void setOffset(const int offset) { m_offset = offset; }
	int offset() const { return m_offset; }

	std::vector<Token> consume(const std::string &data, const std::string &filename, const bool isEnd = true);
	static std::vector<Token> cleanComments(const std::vector<Token> &tokens);

private:
	std::string consumeWhile(Util::SelfContainedIterator<std::string> &it, const std::function<bool(const char)> &isAcceptedCallback);

	StartState m_state = Normal;
	int m_offset = 0;

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
