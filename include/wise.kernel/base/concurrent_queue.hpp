#pragma once 

#include "config.hpp"
#include <memory>

#if WISE_ENABLE_CONCURRENT_QUEUE 
	#include <concurrent_queue.h>
#else
	#include <deque>
	#include <mutex>
#endif

namespace wise
{

template <typename T, typename Alloc = std::allocator<T> >
class concurrent_queue
{
public:
	concurrent_queue();
	~concurrent_queue();

	void push(T v);

	bool pop(T& v);

	bool empty() const;

	void clear()
	{
		q_.clear();
	}

	/// get size. note: thread unsafe. 
	std::size_t unsafe_size() const;

private: 
#if WISE_ENABLE_CONCURRENT_QUEUE
	concurrency::concurrent_queue<T, Alloc> q_;
#else
	std::deque<T, Alloc> q_;
	mutable std::recursive_mutex mutex_;
#endif
};

} // wise

#include "concurrent_queue.inl"
