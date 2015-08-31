#pragma once

#include <stdexcept>

namespace Argonauts {
namespace Util {

class Exception : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

}
}
