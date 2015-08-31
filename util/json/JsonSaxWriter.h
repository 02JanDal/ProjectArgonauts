/*
 * Copyright 2015 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "util/SaxSink.h"

namespace Argonauts {
namespace Util {
class OutputStream
{
public:
	virtual ~OutputStream() {}
	virtual bool write(const std::string &string) = 0;
	virtual bool write(const char c) { return write(std::string({c})); }
};
class StringOutputStream : public OutputStream
{
	std::string *m_str;
	const bool m_owned;
public:
	explicit StringOutputStream(std::string *string = nullptr)
		: m_str(string ? string : new std::string()), m_owned(!string) {}
	~StringOutputStream()
	{
		if (m_owned) {
			delete m_str;
		}
	}
	inline bool write(const std::string &string) override { *m_str += string; return true; }
	const std::string result() const { return *m_str; }
};

namespace Json {
class SaxWriter : public SaxSink
{
	enum ContainerType { Object, Array };

	OutputStream *m_stream;
	const unsigned char m_intendention;
	std::vector<ContainerType> m_containerStack;
	std::vector<bool> m_haveHadValueStack;
public:
	explicit SaxWriter(OutputStream *stream, const unsigned char intendention = 0);

private:
	bool null() override;
	bool boolean(const bool val) override;
	bool integerNumber(const int64_t val) override;
	bool doubleNumber(const double val) override;
	bool string(const std::string &val) override;
	bool startObject() override;
	bool key(const std::string &val) override;
	bool endObject(const std::size_t) override;
	bool startArray() override;
	bool endArray(const std::size_t) override;

private:
	std::string delimiter(const bool isKey = false);
};
}
}
}
