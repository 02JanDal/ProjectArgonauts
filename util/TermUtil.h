#pragma once

#include <string>

namespace TermUtil
{
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
}
