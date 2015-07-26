#include "Importer.h"

#include <iostream>

#include "util/JsonSax.h"
#include "util/StringUtil.h"

class DebuggingJsonHandler : public JsonSaxHandler
{
	bool null()
	{
		std::cout << std::string(indention, ' ') << "Null()" << std::endl;
		return true;
	}
	bool boolean(const bool value)
	{
		std::cout << std::string(indention, ' ') << "Boolean(" << (value ? "true" : "false") << ")" << std::endl;
		return true;
	}
	bool integerNumber(const int64_t value)
	{
		std::cout << std::string(indention, ' ') << "Integer(" << value << ")" << std::endl;
		return true;
	}
	bool doubleNumber(const double value)
	{
		std::cout << std::string(indention, ' ') << "Double(" << value << ")" << std::endl;
		return true;
	}
	bool string(const char *str, const std::size_t size)
	{
		std::cout << std::string(indention, ' ') << "String(\"" << StringUtil::replaceAll(std::string(str, size), "\n", "\\n") << "\")" << std::endl;
		return true;
	}
	bool startObject()
	{
		std::cout << std::string(indention, ' ') << "Object(" << std::endl;
		indention += 4;
		return true;
	}
	bool key(const char *str, const std::size_t size)
	{
		std::cout << std::string(indention, ' ') << "Key(" << std::string(str, size) << ')' << std::endl;
		return true;
	}
	bool endObject(const std::size_t size)
	{
		indention -= 4;
		std::cout << std::string(indention, ' ') << ")Object(" << size << ")" << std::endl;
		return true;
	}
	bool startArray()
	{
		std::cout << std::string(indention, ' ') << "Array(" << std::endl;
		indention += 4;
		return true;
	}
	bool endArray(const std::size_t size)
	{
		indention -= 4;
		std::cout << std::string(indention, ' ') << ")Array(" << size << ")" << std::endl;
		return true;
	}

	int indention = 0;
};

Importer::Importer()
{
}

bool Importer::run()
{
	const char *json = R"json(
{ name: "Ford Prefect\n",
	age: +42,
	alive: false,
	"hobbies": [
		"Hitchhiking",
	]}
)json";
	JsonSax sax(new DebuggingJsonHandler);
	sax.addData(std::string(json));
	sax.end();
	if (sax.isError())
	{
		std::cerr << sax.error() << std::endl;
		return false;
	}
	return true;
}
