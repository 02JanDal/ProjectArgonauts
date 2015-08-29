#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <stdlib.h>

int main(int argc, char *const*argv)
{
	putenv(std::strcpy(new char[16], "TERM_WIDTH=76"));
	return Catch::Session().run(argc, argv);
}
