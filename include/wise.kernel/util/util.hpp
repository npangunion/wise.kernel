#pragma once 
#include <wise.kernel/core/tick.hpp>

#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <math.h>

namespace wise {
namespace kernel {

static constexpr float epsilon = 0.00001f;

template <typename T>
bool is_equal(T a, T b)
{
	return std::abs(a - b) < epsilon;
}

inline void sleep(tick_t ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


inline 
errno_t memcpy(void* dest, std::size_t dest_len, void* src, std::size_t src_len)
{
#ifdef _MSC_VER
	return ::memcpy_s(dest, dest_len, src, src_len);
#else
	::memcpy(dest, src, src_len)); // returns void*

	return 0;
#endif
}

inline
errno_t memmove_s(
	void* dest,
	std::size_t dst_len,
	void* src,
	std::size_t src_len
	)
{
#ifdef _MSC_VER
	return ::memmove_s(dest, dst_len, src, src_len);
#else
	std::memmove(dest, src, src_len)); // returns void*
	return 0;
#endif
}

std::vector<std::string> split(
	const std::string& str,
	const std::string& delims = "\t\n ",
	unsigned int max_splits = 0);

std::string remove_file_extension(const std::string& path);

std::string get_filename(const std::string& path);

} // kernel
} // wise