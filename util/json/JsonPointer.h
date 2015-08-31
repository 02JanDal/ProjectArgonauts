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
#include <stdexcept>

namespace Argonauts {
namespace Util {
namespace Json {
class Value;

class Pointer
{
	bool m_isValid;
	std::vector<std::string> m_referenceTokens;
public:
	/// Attempts to parse and construct a pointer from the given @p pointer. @sa Use Pointer::isValid to check if the pointer could be constructed successfully.
	explicit Pointer(const std::string &pointer);
	/// Constructs an empty, invalid pointer
	explicit Pointer();

	/// @returns true if this is a wellformed, usable pointer
	bool isValid() const { return m_isValid; }

	/// @returns The processed list of reference tokens
	const std::vector<std::string> &referenceTokens() const { return m_referenceTokens; }

	/**
	 * @brief @p evaluate returns that value pointed in @p document that is pointed to by this pointer
	 * @param document The root document from which to begin evaluation
	 * @returns The @p Value that this pointer points at
	 * @throws EvaluationError if the pointer is invalid, or the no value corresponding to the pointer can be found
	 */
	const Value evaluate(const Value &document) const;

	class EvaluationError : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
};
}
}
}
