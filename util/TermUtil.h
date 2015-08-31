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

namespace Argonauts {
namespace Util {
namespace Term {

enum Color
{
	Grey, Red, Green, Yellow, Blue, Magenta, Cyan, White
};
enum Style
{
	Bold, Dark, Underline, Blink, Reverse, Concealed
};

std::string style(const Style style, const std::string &in = std::string());
std::string fg(const Color color, const std::string &in = std::string());
std::string bg(const Color color, const std::string &in = std::string());
std::string reset();
bool isTty();

int currentWidth();

}
}
}
