#include "FSUtil.h"

#include <fstream>
#include <istream>
#include <ostream>

#include "ArgonautsException.h"

std::string FSUtil::readFile(const std::string &filename)
{
	std::ifstream stream;
	stream.open(filename);
	if (!stream.is_open() || !stream.good()) {
		throw ArgonautsException(std::string("Unable to open file '") + filename + "' for reading");
	}
	stream.seekg(0, std::ios_base::end);
	const int length = stream.tellg();
	stream.seekg(0, std::ios_base::beg);
	char buf[length];
	stream.read(buf, length);
	stream.close();
	return std::string(buf, length);
}

void FSUtil::writeFile(const std::string &filename, const std::string &data)
{
	std::ofstream stream;
	stream.open(filename);
	if (!stream.is_open() || !stream.good()) {
		throw ArgonautsException(std::string("Unable to open file '") + filename + "' for writing");
	}
	stream << data;
	stream.close();
}
