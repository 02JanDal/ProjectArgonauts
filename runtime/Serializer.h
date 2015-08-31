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

#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "util/SaxSink.h"

namespace std
{
template <typename A, typename B> class map;
}

namespace Argonauts
{
namespace Runtime
{
template <typename T>
void serialize(const T &t, class Serializer *serializer) { t.serialize(serializer); }

class Serializer
{
public:
	virtual ~Serializer();

	virtual bool packedEnums() const { return false; }

	virtual void emitValue(const std::string &val) = 0;
	virtual void emitValue(const std::int8_t val) { return emitValue(std::int64_t(val)); }
	virtual void emitValue(const std::int16_t val) { return emitValue(std::int64_t(val)); }
	virtual void emitValue(const std::int32_t val) { return emitValue(std::int64_t(val)); }
	virtual void emitValue(const std::int64_t val) = 0;
	virtual void emitValue(const std::uint8_t val) { return emitValue(std::int64_t(val)); }
	virtual void emitValue(const std::uint16_t val) { return emitValue(std::int64_t(val)); }
	virtual void emitValue(const std::uint32_t val) { return emitValue(std::int64_t(val)); }
	virtual void emitValue(const std::uint64_t val) { return emitValue(std::int64_t(val)); }
	virtual void emitValue(const double val) = 0;
	virtual void emitArrayStart() = 0;
	virtual void emitArrayEnd(const std::size_t size) = 0;
	virtual void emitObjectStart() = 0;
	virtual void emitObjectKey(const std::string &key) = 0;
	virtual void emitObjectEnd(const std::size_t size) = 0;

	template <typename Type>
	void emitValue(const std::map<std::string, Type> &map)
	{
		emitObjectValue(map.begin(), map.end(), map.size());
	}
	template <typename Type>
	void emitValue(const std::unordered_map<std::string, Type> &map)
	{
		emitObjectValue(map.begin(), map.end(), map.size());
	}

	template <typename Type>
	void emitValue(const std::vector<Type> &array)
	{
		emitArrayStart();
		for (const auto &item : array) {
			emitValue(item);
		}
		emitArrayEnd(array.size());
	}

	template <typename Type>
	void emitValue(const Type &t)
	{
		serialize(t, this);
	}

private:
	template <typename Iterator>
	inline void emitObjectValue(Iterator begin, Iterator end, const std::size_t size)
	{
		emitObjectStart();
		for (auto it = begin; it != end; ++it) {
			emitObjectKey((*it).first);
			emitValue((*it).second);
		}
		emitObjectEnd(size);
	}
};

class SaxSinkSerializer : public Serializer
{
	Argonauts::Util::SaxSink *m_sink;

public:
	explicit SaxSinkSerializer(Argonauts::Util::SaxSink *sink) : m_sink(sink) {}

	void emitValue(const std::string &val) override { m_sink->string(val); }
	void emitValue(const std::int64_t val) override { m_sink->integerNumber(val); }
	void emitValue(const double val) override { m_sink->doubleNumber(val); }
	void emitArrayStart() override { m_sink->startArray(); }
	void emitArrayEnd(const std::size_t size) override { m_sink->endArray(size); }
	void emitObjectStart() override { m_sink->startObject(); }
	void emitObjectKey(const std::string &key) override { m_sink->key(key); }
	void emitObjectEnd(const std::size_t size) override { m_sink->endObject(size); }
};
}
}
