#pragma once

#include <string>
#include <vector>

class Importer
{
public:
	explicit Importer();

	std::vector<std::string> schemas;
	std::string output;
	bool force = false;

	bool run();
};
