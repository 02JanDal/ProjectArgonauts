#pragma once

#include <string>

class Compiler
{
public:
	explicit Compiler();

	std::string input;
	std::string output;

	bool run();
};
