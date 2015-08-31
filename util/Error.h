#pragma once

#include <stdexcept>
#include <string>

namespace Argonauts {
namespace Util {

class Error : public std::runtime_error
{
public:
	class Source
	{
		friend class Error;
		const std::string m_data;
		const std::string m_filename;
	public:
		explicit Source(const std::string &data, const std::string &filename = "<unknown>");
		explicit Source();
	};

	explicit Error();
	explicit Error(const std::string &msg, const std::size_t offset = std::string::npos, const Source &source = Source());

	std::pair<long, long> lineColumnFromDataAndOffset() const;
	std::string lineInData(const int offset = 0) const;
	std::string errorMessage() const;

private:
	const std::string m_error;
	const std::size_t m_offset = std::string::npos;
	const Source m_source;
};

}
}
