#pragma once

#include <string>

namespace Argonauts {
namespace Util { class SaxSink; }
namespace Runtime {

struct HandleParseAction
{
	enum Action
	{
		Error,
		DelegateToArray,
		DelegateToObject,
		Success
	} action;

	Util::SaxSink *delegationTarget;
	std::string error;

	HandleParseAction(const Action &a) : action(a) {}
	explicit HandleParseAction(const Action &a, Util::SaxSink *sink) : action(a), delegationTarget(sink) {}
	explicit HandleParseAction(const std::string &err) : action(Error), error(err) {}
};

template <typename Type> HandleParseAction handleParseNull(Type &) { return HandleParseAction("Unexpected value of type 'null'"); }
template <typename Type> HandleParseAction handleParseInteger(Type &type, const int64_t val);
template <typename Type> HandleParseAction handleParseDouble(Type &type, const double val);
template <typename Type> HandleParseAction handleParseString(Type &type, const std::string &string);
template <typename Type> HandleParseAction handleParseObject(Type &) { return HandleParseAction("Unexpected value of type 'object'"); }
template <typename Type> HandleParseAction handleParseArray(Type &type);

inline HandleParseAction handleParseInteger(int8_t &type, const int64_t val) { type = int8_t(val); return HandleParseAction::Success; }
inline HandleParseAction handleParseInteger(int16_t &type, const int64_t val) { type = int16_t(val); return HandleParseAction::Success; }
inline HandleParseAction handleParseInteger(int32_t &type, const int64_t val) { type = int32_t(val); return HandleParseAction::Success; }
inline HandleParseAction handleParseInteger(int64_t &type, const int64_t val) { type = int64_t(val); return HandleParseAction::Success; }
inline HandleParseAction handleParseInteger(uint8_t &type, const int64_t val) { type = uint8_t(val); return HandleParseAction::Success; }
inline HandleParseAction handleParseInteger(uint16_t &type, const int64_t val) { type = uint16_t(val); return HandleParseAction::Success; }
inline HandleParseAction handleParseInteger(uint32_t &type, const int64_t val) { type = uint32_t(val); return HandleParseAction::Success; }
inline HandleParseAction handleParseInteger(uint64_t &type, const int64_t val) { type = uint64_t(val); return HandleParseAction::Success; }
inline HandleParseAction handleParseInteger(double &type, const int64_t val) { type = val; return HandleParseAction::Success; }
inline HandleParseAction handleParseDouble(double &type, const double val) { type = val; return HandleParseAction::Success; }
inline HandleParseAction handleParseString(std::string &type, const std::string &string) { type = string; return HandleParseAction::Success; }

}
}
