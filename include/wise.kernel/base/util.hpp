#pragma once

#include <wise/base/tick.hpp>

#include <string>
#include <chrono>
#include <thread>
#include <vector>

namespace wise
{

const float eps = 0.00001f;

inline void sleep(tick_t ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline bool is_equal(float f1, float f2)
{
	return std::abs(f1 - f2) < eps;
}

inline bool is_equal(double d1, double d2)
{
	return std::abs(d1 - d2) < eps;
}

inline errno_t memcpy(void* dest, std::size_t dest_len, void* src, std::size_t src_len)
{
#ifdef _MSC_VER
	return ::memcpy_s(dest, dest_len, src, src_len);
#else
	::memcpy(dest, src, src_len)); // returns void*

	return 0;
#endif
}

std::vector<std::string> split(
	const std::string& str,
	const std::string& delims = "\t\n ",
	unsigned int max_splits = 0);

std::string remove_file_extension(const std::string& path);

std::string get_filename(const std::string& path);

uint32_t get_crc32(const std::string& data);

uint32_t get_crc32(const char* data, std::size_t length);

} // wise