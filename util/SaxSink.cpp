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

#include "SaxSink.h"

#include "Util.h"

namespace Argonauts {
namespace Util {

bool DelegatingSaxSink::null()
{
	if (m_delegatingTo) {
		return reportErrorIfFalse(m_delegatingTo->null());
	} else {
		return nullImpl();
	}
}
bool DelegatingSaxSink::boolean(const bool val)
{
	if (m_delegatingTo) {
		return reportErrorIfFalse(m_delegatingTo->boolean(val));
	} else {
		return booleanImpl(val);
	}
}
bool DelegatingSaxSink::integerNumber(const int64_t val)
{
	if (m_delegatingTo) {
		return reportErrorIfFalse(m_delegatingTo->integerNumber(val));
	} else {
		return integerNumberImpl(val);
	}
}
bool DelegatingSaxSink::doubleNumber(const double val)
{
	if (m_delegatingTo) {
		return reportErrorIfFalse(m_delegatingTo->doubleNumber(val));
	} else {
		return doubleNumberImpl(val);
	}
}
bool DelegatingSaxSink::string(const std::string &str)
{
	if (m_delegatingTo) {
		return reportErrorIfFalse(m_delegatingTo->string(str));
	} else {
		return stringImpl(str);
	}
}
bool DelegatingSaxSink::startObject()
{
	++m_nestingLevelCounter;
	if (m_delegatingTo) {
		return reportErrorIfFalse(m_delegatingTo->startObject());
	} else {
		return startObjectImpl();
	}
}
bool DelegatingSaxSink::key(const std::string &str)
{
	if (m_delegatingTo) {
		return reportErrorIfFalse(m_delegatingTo->key(str));
	} else {
		return keyImpl(str);
	}
}
bool DelegatingSaxSink::endObject(const std::size_t size)
{
	if (m_delegatingTo) {
		const bool res = reportErrorIfFalse(m_delegatingTo->endObject(size));
		--m_nestingLevelCounter;
		if (m_nestingLevelCounter == 0)
		{
			delete m_delegatingTo;
			m_delegatingTo = nullptr;
		}
		return res;
	} else {
		return endObjectImpl(size);
	}
}
bool DelegatingSaxSink::startArray()
{
	++m_nestingLevelCounter;
	if (m_delegatingTo) {
		return reportErrorIfFalse(m_delegatingTo->startArray());
	} else {
		return startArrayImpl();
	}
}
bool DelegatingSaxSink::endArray(const std::size_t size)
{
	if (m_delegatingTo) {
		const bool res = reportErrorIfFalse(m_delegatingTo->endArray(size));
		--m_nestingLevelCounter;
		if (m_nestingLevelCounter == 0)
		{
			delete m_delegatingTo;
			m_delegatingTo = nullptr;
		}
		return res;
	} else {
		return endArrayImpl(size);
	}
}

bool DelegatingSaxSink::reportErrorIfFalse(const bool value)
{
	if (!value) {
		reportError(m_delegatingTo->error().c_str());
	}
	return value;
}

void DelegatingSaxSink::delegateToArray(SaxSink *handler)
{
	ASSERT(handler);
	m_delegatingTo = handler;
	m_delegatingTo->startArray();
	m_nestingLevelCounter = 1;
}
void DelegatingSaxSink::delegateToObject(SaxSink *handler)
{
	ASSERT(handler);
	m_delegatingTo = handler;
	m_delegatingTo->startObject();
	m_nestingLevelCounter = 1;
}

}
}
