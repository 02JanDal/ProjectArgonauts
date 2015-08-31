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

#include "TermUtil.h"

#include "OsUtil.h"

#ifdef OS_UNIX
# include <sys/ioctl.h>
# include <cstdio>
# include <unistd.h>
#else
# include <io.h>
# include <windows.h>
# warning Windows support has not yet been implemented. No colors will be shown.
#endif

namespace Argonauts {
namespace Util {
namespace Term {
namespace detail {

#ifdef OS_UNIX
static std::string getStyleCode(const Style style)
{
	switch (style)
	{
	case Term::Bold: return "\033[1m";
	case Term::Dark: return "\033[2m";
	case Term::Underline: return "\033[4m";
	case Term::Blink: return "\033[5m";
	case Term::Reverse: return "\033[7m";
	case Term::Concealed: return "\033[8m";
	}
}
static std::string getStyleEndCode(const Style style)
{
	switch (style)
	{
	case Term::Bold: return "\033[22m";
	case Term::Dark: return "\033[22m";
	case Term::Underline: return "\033[24m";
	case Term::Blink: return "\033[25m";
	case Term::Reverse: return "\033[27m";
	case Term::Concealed: return "\033[38m";
	}
}
static std::string getFGColorCode(const Color color)
{
	switch (color)
	{
	case Term::Grey: return "\033[30m";
	case Term::Red: return "\033[31m";
	case Term::Green: return "\033[32m";
	case Term::Yellow: return "\033[33m";
	case Term::Blue: return "\033[34m";
	case Term::Magenta: return "\033[35m";
	case Term::Cyan: return "\033[36m";
	case Term::White: return "\033[37m";
	}
}
static std::string getBGColorCode(const Color color)
{
	switch (color)
	{
	case Term::Grey: return "\033[40m";
	case Term::Red: return "\033[41m";
	case Term::Green: return "\033[42m";
	case Term::Yellow: return "\033[43m";
	case Term::Blue: return "\033[44m";
	case Term::Magenta: return "\033[45m";
	case Term::Cyan: return "\033[46m";
	case Term::White: return "\033[47m";
	}
}
#endif
}

std::string style(const Style style, const std::string &in)
{
	if (!isTty())
	{
		return in;
	}
#ifdef OS_WINDOWS
	return in;
#else
	if (in.empty())
	{
		return detail::getStyleCode(style);
	}
	else
	{
		return detail::getStyleCode(style) + in + detail::getStyleEndCode(style);
	}
#endif
}
std::string fg(const Color color, const std::string &in)
{
	if (!isTty())
	{
		return in;
	}
#ifdef OS_WINDOWS
	return in;
#else
	if (in.empty())
	{
		return detail::getFGColorCode(color);
	}
	else
	{
		return detail::getFGColorCode(color) + in + "\033[39m";
	}
#endif
}
std::string bg(const Color color, const std::string &in)
{
	if (!isTty())
	{
		return in;
	}
#ifdef OS_WINDOWS
	return in;
#else
	if (in.empty())
	{
		return detail::getBGColorCode(color);
	}
	else
	{
		return detail::getBGColorCode(color) + in + "\033[39m";
	}
#endif
}
std::string reset()
{
	if (!isTty())
	{
		return std::string();
	}
#ifdef OS_WINDOWS
	return "";
#else
	return "\033[00m";
#endif
}

bool isTty()
{
#ifdef OS_WINDOWS
	return ::_isatty(_fileno(stdout));
#else
	return ::isatty(fileno(stdout));
#endif
}

int currentWidth()
{
#ifdef OS_UNIX
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
#else
	return 120;
#endif
}

}
}
}
