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

#include <vector>
#include <utility>
#include <functional>

namespace Argonauts {
namespace Common {
class Token;

class GeneralParser
{
public:
	using ReduceCommand = std::pair<int, std::vector<Token>>;
	using ReduceFunction = std::function<ReduceCommand(const Token &token, const std::vector<Token> &stack)>;
	using ShiftFunction = std::function<void(const std::vector<Token> &stack)>;
	using MorphFunction = std::function<Token(const Token &token)>;
	using UnexpectedFunction = std::function<void(const Token &token)>;

	// the order of reduce functions: 0: defineStruct, 1: defineStructWithInclude, 2: defineEnum, 3: defineUsing,
	//		4: defineAnnotation, 5: addToAnnotation, 6: defineEnumEntry, 7: defineStructAttribute,
	//		8: defineStructAttributeWithIndex, 9: convertTypeToTypeList, 10: mergeTypeListAndType, 11: finalizeType
	// the order of shift functions: 0: typeDefinition, 1: typeUsage
	// the order of morph functions: 0: Identifier->Type, 1: Type->TypeList
	static std::vector<Token> parse(const std::vector<ReduceFunction> &reducers,
									const std::vector<ShiftFunction> &shifters,
									const std::vector<MorphFunction> &morphers,
									const std::vector<Token> &startStack,
									const std::vector<Token> tokens,
									UnexpectedFunction unexpected);

public: // default functions
	static ReduceCommand defineStruct(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand defineStructWithInclude(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand defineEnum(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand defineUsing(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand defineAnnotation(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand addToAnnotation(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand defineEnumEntry(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand defineStructAttribute(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand defineStructAttributeWithIndex(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand convertTypeToTypeList(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand mergeTypeListAndType(const Token &token, const std::vector<Token> &stack);
	static ReduceCommand finalizeType(const Token &token, const std::vector<Token> &stack);
};
}
}
