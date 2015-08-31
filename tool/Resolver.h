#pragma once

#include "DataTypes.h"

namespace Argonauts {
namespace Tool {

class Resolver
{
public:
	explicit Resolver(const File &file);

	class ResolverError : public std::runtime_error
	{
	public:
		explicit ResolverError(const std::string &msg, const int offset_) : std::runtime_error(msg), offset(offset_) {}

		const int offset;
	};

	File result() const { return m_file; }
	void resolveAliases();
	void resolveIncludes();

private:
	File m_file;
};

}
}
