#pragma once

#if defined(_WIN32) || defined(_WIN64)
# define OS_WINDOWS
#elif defined(__APPLE__)
# define OS_OSX
#elif defined(linux) || defined(__linux)
# define OS_LINUX
#else
# error Unknown platform
#endif

#if defined(OS_OSX) || defined(OS_LINUX)
# define OS_UNIX
#endif
