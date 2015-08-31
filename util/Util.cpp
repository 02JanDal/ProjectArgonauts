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

#include "Util.h"

#include <iostream>

void Argonauts::Util::assert(const bool condition, const char *condString, const char *file, const int line, const std::string &message)
{
	if (!condition) {
		std::cerr << "Assertion failed: " << condString << " == false at " << file << ":" << line;
		if (!message.empty()) {
			std::cerr << " (" << message << ")";
		}
		std::cerr << "\n" << std::flush;
		std::abort();
	}
}
