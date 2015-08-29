#pragma once

#include <iterator>

// mostly taken from Q_DECLARE_SEQUENTIAL_ITERATOR
template <typename T>
class SelfContainedIterator
{
	using const_iterator = typename T::const_iterator;
	using Type = typename std::iterator_traits<const_iterator>::value_type;

	const T container;
	const_iterator it;

public:
	inline explicit SelfContainedIterator(const T &c)
		: container(c), it(std::begin(c)) {}
	inline void toFront() { it = std::begin(container); }
	inline void toBack() { it = std::end(container); }
	inline bool hasNext() const { return it != std::end(container); }
	inline bool hasPrevious() const { return it != std::begin(container); }
	inline Type next() { return *it++; }
	inline Type previous() { return *--it; }
	inline Type peekNext() const { return *it; }
	inline Type peekPrevious() const { const_iterator p = it; return *--p; }
	inline bool findNext(const T &t) { while (it != std::end(container)) if (*it++ == t) return true; return false; }
	inline bool findPrevious(const T &t) { while (it != std::begin(container)) if (*(--it) == t) return true; return false; }
	inline int index() const { const_iterator p = it; return (--p) - std::begin(container); }
};
