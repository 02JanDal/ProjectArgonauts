#include "JsonSaxWriter.h"

#include "StringUtil.h"

static inline std::string replaceEscaped(const std::string &in)
{
	using Argonauts::Util::String::replaceAll;
	std::string out = in;
	out = replaceAll(out, "\\", "\\\\");
	out = replaceAll(out, "/", "\\/");
	out = replaceAll(out, "\"", "\\\"");
	out = replaceAll(out, "\b", "\\b");
	out = replaceAll(out, "\f", "\\f");
	out = replaceAll(out, "\n", "\\n");
	out = replaceAll(out, "\r", "\\r");
	out = replaceAll(out, "\t", "\\t");
	return out;
}

namespace Argonauts {
namespace Util {
namespace Json {
SaxWriter::SaxWriter(OutputStream *stream, const unsigned char intendention)
	: m_stream(stream), m_intendention(intendention) {}

bool SaxWriter::null()
{
	return m_stream->write(delimiter() + "null");
}
bool SaxWriter::boolean(const bool val)
{
	return m_stream->write(delimiter() + (val ? "true" : "false"));
}
bool SaxWriter::integerNumber(const int64_t val)
{
	return m_stream->write(delimiter() + std::to_string(val));
}
bool SaxWriter::doubleNumber(const double val)
{
	std::string value = std::to_string(val);
	std::size_t pos;
	// remove trailing zeros
	while ((pos = value.rfind('0')) == (value.size() - 1)) {
		value = value.substr(0, value.size() - 1);
	}
	return m_stream->write(delimiter() + value);
}
bool SaxWriter::string(const std::string &val)
{
	return m_stream->write(delimiter() + '"' + replaceEscaped(val) + '"');
}
bool SaxWriter::startObject()
{
	const bool res = m_stream->write(delimiter() + '{');
	m_containerStack.push_back(Object);
	m_haveHadValueStack.push_back(false);
	return res;
}
bool SaxWriter::key(const std::string &val)
{
	return m_stream->write(delimiter(true) + '"' + replaceEscaped(val) + "\":" + (m_intendention == 0 ? "" : " "));
}
bool SaxWriter::endObject(const std::size_t)
{
	m_haveHadValueStack.pop_back();
	m_containerStack.pop_back();
	return m_stream->write('}');
}
bool SaxWriter::startArray()
{
	const bool res = m_stream->write(delimiter() + '[');
	m_containerStack.push_back(Array);
	m_haveHadValueStack.push_back(false);
	return res;
}
bool SaxWriter::endArray(const std::size_t)
{
	m_haveHadValueStack.pop_back();
	m_containerStack.pop_back();
	return m_stream->write(']');
}

std::string SaxWriter::delimiter(const bool isKey)
{
	if (m_haveHadValueStack.empty())
	{
		return std::string();
	}

	if (isKey && m_containerStack.back() == Object && m_haveHadValueStack.back())
	{
		// object, key and have had a previous value
		return ",";
	}
	else if (!isKey && m_containerStack.back() == Array && m_haveHadValueStack.back())
	{
		// array and have had a previous value
		return ",";
	}
	else
	{
		m_haveHadValueStack.back() = true;
		return std::string();
	}
}
}
}
}
