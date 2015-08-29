#pragma once

#include <string>
#include <vector>

namespace Argonauts {
namespace Util {

class SaxSink
{
public:
	virtual ~SaxSink() {}

	virtual bool null() = 0;
	virtual bool boolean(const bool val) = 0;
	virtual bool integerNumber(const int64_t val) = 0;
	virtual bool doubleNumber(const double val) = 0;
	virtual bool string(const std::string &str) = 0;
	virtual bool startObject() = 0;
	virtual bool key(const std::string &str) = 0;
	virtual bool endObject(const std::size_t size) = 0;
	virtual bool startArray() = 0;
	virtual bool endArray(const std::size_t size) = 0;

	std::string error() const { return m_error; }

protected:
	template <typename... Args>
	bool reportError(const char *str, Args&&... args)
	{
		char buf[512];
		std::snprintf(buf, 512, str, std::forward<Args>(args)...);
		m_error = buf;
		return false;
	}
	bool reportError(const std::string &str)
	{
		m_error = str;
		return false;
	}

private:
	std::string m_error;
};

class DelegatingSaxSink : public SaxSink
{
	SaxSink *m_delegatingTo = nullptr;
	int m_nestingLevelCounter = 0;
public:
	virtual ~DelegatingSaxSink() {}

private:
	bool null() override final;
	bool boolean(const bool val) override final;
	bool integerNumber(const int64_t val) override final;
	bool doubleNumber(const double val) override final;
	bool string(const std::string &str) override final;
	bool startObject() override final;
	bool key(const std::string &str) override final;
	bool endObject(const std::size_t size) override final;
	bool startArray() override final;
	bool endArray(const std::size_t size) override final;

	bool reportErrorIfFalse(const bool value);

protected:
	virtual bool nullImpl() = 0;
	virtual bool booleanImpl(const bool val) = 0;
	virtual bool integerNumberImpl(const int64_t val) = 0;
	virtual bool doubleNumberImpl(const double val) = 0;
	virtual bool stringImpl(const std::string &str) = 0;
	virtual bool startObjectImpl() = 0;
	virtual bool keyImpl(const std::string &str) = 0;
	virtual bool endObjectImpl(const std::size_t size) = 0;
	virtual bool startArrayImpl() = 0;
	virtual bool endArrayImpl(const std::size_t size) = 0;

	void delegateToObject(SaxSink *handler);
	void delegateToArray(SaxSink *handler);
};
}
}
