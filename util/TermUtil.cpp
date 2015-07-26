#include "TermUtil.h"

#include "OsUtil.h"

#ifdef OS_UNIX
# include <unistd.h>
#else
# include <io.h>
# include <windows.h>
# warning Windows support has not yet been implemented. No colors will be shown.
#endif

namespace TermUtil
{
namespace detail
{
#ifdef OS_UNIX
std::string getStyleCode(const Style style)
{
	switch (style)
	{
	case TermUtil::Bold: return "\033[1m";
	case TermUtil::Dark: return "\033[2m";
	case TermUtil::Underline: return "\033[4m";
	case TermUtil::Blink: return "\033[5m";
	case TermUtil::Reverse: return "\033[7m";
	case TermUtil::Concealed: return "\033[8m";
	}
}
std::string getStyleEndCode(const Style style)
{
	switch (style)
	{
	case TermUtil::Bold: return "\033[22m";
	case TermUtil::Dark: return "\033[22m";
	case TermUtil::Underline: return "\033[24m";
	case TermUtil::Blink: return "\033[25m";
	case TermUtil::Reverse: return "\033[27m";
	case TermUtil::Concealed: return "\033[38m";
	}
}
std::string getFGColorCode(const Color color)
{
	switch (color)
	{
	case TermUtil::Grey: return "\033[30m";
	case TermUtil::Red: return "\033[31m";
	case TermUtil::Green: return "\033[32m";
	case TermUtil::Yellow: return "\033[33m";
	case TermUtil::Blue: return "\033[34m";
	case TermUtil::Magenta: return "\033[35m";
	case TermUtil::Cyan: return "\033[36m";
	case TermUtil::White: return "\033[37m";
	}
}
std::string getBGColorCode(const Color color)
{
	switch (color)
	{
	case TermUtil::Grey: return "\033[40m";
	case TermUtil::Red: return "\033[41m";
	case TermUtil::Green: return "\033[42m";
	case TermUtil::Yellow: return "\033[43m";
	case TermUtil::Blue: return "\033[44m";
	case TermUtil::Magenta: return "\033[45m";
	case TermUtil::Cyan: return "\033[46m";
	case TermUtil::White: return "\033[47m";
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

}
