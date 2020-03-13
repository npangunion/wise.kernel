#include "concurrent_queue.hpp"

namespace wise {
namespace kernel {

template <typename T, typename Alloc>
concurrent_queue<T, Alloc>::concurrent_queue()
{
}

template <typename T, typename Alloc>
concurrent_queue<T, Alloc>::~concurrent_queue()
{
}

#if WISE_ENABLE_CONCURRENT_QUEUE  

template <typename T, typename Alloc>
void concurrent_queue<T, Alloc>::push(T v)
{
	q_.push(std::move(v));
}

template <typename T, typename Alloc>
bool concurrent_queue<T, Alloc>::pop(T& v)
{
	return q_.try_pop(v);
}

template <typename T, typename Alloc>
bool concurrent_queue<T, Alloc>::empty() const
{
	return q_.empty();
}

template <typename T, typename Alloc>
std::size_t concurrent_queue<T, Alloc>::unsafe_size() const
{
	return q_.unsafe_size();
}

#else 
template <typename T, typename Alloc>
void concurrent_queue<T, Alloc>::push(T v)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);
	q_.push_back(std::move(v));
}

template <typename T, typename Alloc>
bool concurrent_queue<T, Alloc>::pop(T& v)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	if (q_.empty())
	{
		return false;
	}

	v = q_.front();
	q_.pop_front();

	return true;
}

template <typename T, typename Alloc>
bool concurrent_queue<T, Alloc>::empty() const
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);
	return q_.empty();
}

template <typename T, typename Alloc>
std::size_t concurrent_queue<T, Alloc>::unsafe_size() const
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	return  q_.size();
}

#endif

}
} // wise::kernel
