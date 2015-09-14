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

#include <type_traits>
#include <stdexcept>

namespace Argonauts {
namespace Util {
namespace detail {

template <std::size_t N, typename... Types>
struct TypeHelper;
template <std::size_t N, typename T, typename... Types>
struct TypeHelper<N, T&, Types...>
{
	static inline void doDelete(std::size_t, void *) {}
	static inline void doCopy(std::size_t, const void *, void *) {}
	static inline void doMove(std::size_t, void *, void *) {}
	static inline bool doCompare(std::size_t, const void *, const void *) {}
};
template <std::size_t N, typename T, typename... Types>
struct TypeHelper<N, T, Types...>
{
	static inline void doDelete(std::size_t index, void *data)
	{
		if (index == N) {
			reinterpret_cast<T *>(data)->~T();
		} else {
			TypeHelper<N + 1, Types...>::doDelete(index, data);
		}
	}
	static inline void doCopy(std::size_t index, const void *oldData, void *newData)
	{
		if (index == N) {
			new (newData) T(*reinterpret_cast<const T *>(oldData));
		} else {
			TypeHelper<N + 1, Types...>::doCopy(index, oldData, newData);
		}
	}
	static inline void doMove(std::size_t index, void *oldData, void *newData)
	{
		if (index == N) {
			new (newData) T(std::move(*reinterpret_cast<T *>(oldData)));
		} else {
			TypeHelper<N + 1, Types...>::doMove(index, oldData, newData);
		}
	}
	static inline bool doCompare(std::size_t index, const void *a, const void *b)
	{
		if (index == N) {
			return *reinterpret_cast<const T *>(a) == *reinterpret_cast<const T *>(b);
		} else {
			return TypeHelper<N + 1, Types...>::doCompare(index, a, b);
		}
	}
};
template <std::size_t N>
struct TypeHelper<N>
{
	static inline void doDelete(std::size_t, void *)
	{
		throw std::logic_error("Invalid variant index");
	}
	static inline void doCopy(std::size_t, const void *, void *)
	{
		throw std::logic_error("Invalid variant index");
	}
	static inline void doMove(std::size_t, void *, void *)
	{
		throw std::logic_error("Invalid variant index");
	}
	static inline bool doCompare(std::size_t, const void *, const void *)
	{
		throw std::logic_error("Invalid variant index");
	}
};

template <typename Type, typename... Types>
struct Position;
template <typename Type>
struct Position<Type>
{
	static constexpr int index = -1;
};
template <typename Type, typename... Types>
struct Position<Type, Type, Types...>
{
	static constexpr int index = 0;
};
template <typename Needle, typename Type, typename... Types>
struct Position<Needle, Type, Types...>
{
	static constexpr int index = Position<Needle, Types...>::index != -1 ? Position<Needle, Types...>::index + 1 : -1;
};
}

template <typename... Types>
class Variant
{
	typename std::aligned_union<4, Types...>::type m_data;
	std::size_t m_which;

	template <typename T>
	using Position = detail::Position<T, Types...>;
	using TypeHelper = detail::TypeHelper<0, Types...>;

	template <typename T>
	void initialize(const T &t)
	{
		m_which = Position<T>::index;
		new (&m_data) T(t);
	}

	void deinit()
	{
		TypeHelper::doDelete(m_which, reinterpret_cast<void *>(&m_data));
	}

public:
	// constructors and destructors
	template <typename T>
	Variant(const T &t)
	{
		static_assert(Position<T>::index != -1, "The given type does not exist in the variant");
		initialize(t);
	}
	Variant(const Variant<Types...> &original) : m_which(original.m_which)
	{
		TypeHelper::doCopy(m_which, &original.m_data, &m_data);
	}
	Variant(Variant<Types...> &&original) : m_which(original.m_which)
	{
		TypeHelper::doMove(m_which, &original.m_data, &m_data);
	}
	~Variant()
	{
		deinit();
	}

	// assignment
	template <typename T>
	Variant<Types...> &operator=(const T &t)
	{
		static_assert(Position<T>::index != -1, "The given type does not exist in the variant");
		deinit();
		initialize(t);
		return *this;
	}
	Variant<Types...> &operator=(const Variant<Types...> &other)
	{
		deinit();
		m_which = other.m_which;
		TypeHelper::doCopy(m_which, &other.m_data, &m_data);
		return *this;
	}
	Variant<Types...> &operator=(Variant<Types...> &&other)
	{
		std::swap(m_which, other.m_which);
		std::swap(m_data, other.m_data);
		return *this;
	}

	// comparison
	bool operator==(const Variant<Types...> &other) const
	{
		if (m_which != other.m_which) {
			return false;
		} else {
			return TypeHelper::doCompare(m_which, &other.m_data, &m_data);
		}
	}
	bool operator!=(const Variant<Types...> &other) const
	{
		if (m_which != other.m_which) {
			return true;
		} else {
			return !TypeHelper::doCompare(m_which, &other.m_data, &m_data);
		}
	}

	// accessors
	template <typename T>
	T &get()
	{
		static_assert(Position<T>::index != -1, "The given type does not exist in the variant");
		if (m_which == Position<T>::index) {
			return *reinterpret_cast<T *>(&m_data);
		} else {
			throw std::runtime_error(std::string("Variant does currently not contain a value of the requested type"));
		}
	}
	template <typename T>
	const T &get() const
	{
		static_assert(Position<T>::index != -1, "The given type does not exist in the variant");
		if (m_which == Position<T>::index) {
			return *reinterpret_cast<const T *>(&m_data);
		} else {
			throw std::runtime_error(std::string("Variant does currently not contain a value of the requested type"));
		}
	}

	template <typename T>
	bool is() const
	{
		static_assert(Position<T>::index != -1, "The given type does not exist in the variant");
		return m_which == Position<T>::index;
	}
};

}
}
