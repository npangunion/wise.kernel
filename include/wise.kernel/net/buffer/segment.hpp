#pragma once 

#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/util/util.hpp>
#include <array>

namespace wise {
namespace kernel {

/// buffer segment
template <std::size_t Length>
class segment
{
	using container = std::array<uint8_t, Length>;

	class ptr
	{
	public:
		ptr(const uint8_t* data, std::size_t len)
			: data_(data), len_(len)
		{
			WISE_ASSERT(data_);
		}

		const uint8_t* data() const { return data_; }
		std::size_t size() const { return len_; }

	private:
		const uint8_t* data_ = nullptr;
		std::size_t len_ = 0;
	};

public:
	segment() {}
	~segment() {}

	/// p에서 len 만큼 쓰려고 시도하고 쓴 길이만큼 돌려준다. 
	std::size_t append(const uint8_t* p, std::size_t len)
	{
		WISE_RETURN_IF(p == nullptr, 0);
		WISE_RETURN_IF(len == 0, 0);

		WISE_ENSURE(Length >= pos_);

		auto possible_len = std::min(len, Length - pos_);
		WISE_RETURN_IF(possible_len == 0, 0);

		uint8_t* dest = buf_.data() + pos_;

		::memcpy(dest, p, possible_len);

		advance(possible_len);

		return possible_len;
	}

	std::size_t append(const char* p, std::size_t len)
	{
		return append((uint8_t*)p, len);
	}

	std::size_t append(const void* p, std::size_t len)
	{
		return append((uint8_t*)p, len);
	}

	std::size_t write(const char* p, std::size_t len)
	{
		return append(p, len);
	}

	/// get pointer and length
	ptr get() const
	{
		return ptr(buf_.data(), pos_);
	}

	ptr get()
	{
		return ptr(buf_.data(), pos_);
	}

	const uint8_t* data() const
	{
		return buf_.data();
	}

	/// 현재 쓰여진 크기
	std::size_t size() const
	{
		return pos_;
	}

	/// 세그먼트 앞으로 돌린다.
	void rewind()
	{
		pos_ = 0;
	}

	std::size_t capacity() const
	{
		return Length;
	}

	void pop_front(std::size_t count)
	{
		// 안전운전
		count = std::min(count, size());

		memmove_s(
			(void*)buf_.data(), buf_.size(),
			(void*)(buf_.data() + count), pos_ - count
		);

		pos_ -= count;
	}

private:
	void advance(std::size_t len)
	{
		pos_ += len;

		WISE_ASSERT(pos_ <= Length);
	}

private:
	container buf_;
	std::size_t pos_ = 0;
};

} // kernel
} // wise
