#pragma once 

#include <wise.kernel/net/buffer/multiple_size_buffer_pool.hpp>
#include <wise.kernel/net/buffer/resize_buffer_iterator.hpp>
#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/core/exception.hpp>
#include <wise.kernel/util/util.hpp>

#include <algorithm>
#include <vector>

namespace wise {
namespace kernel {

/// increasing size buffer
/**
 * not thread-safe.
 * - needs to use a lock if used from multiple threads
 * - if 1-recv is used, then lock is not required
 */
class resize_buffer
{
public:
	using pool = multiple_size_buffer_pool;

public:
	using value_type = uint8_t;
	using iterator = resize_buffer_iterator<value_type>;
	using const_iterator = resize_buffer_iterator<const value_type>;

	static void clear()
	{
		pool_.clear();
	}

public:

	resize_buffer(std::size_t initial_size = 16 * 1024)
		: initial_size_(initial_size)
	{
		WISE_EXPECT(initial_size_ > 0);
		WISE_EXPECT(pos_ == 0);
		WISE_EXPECT(!buf_);
	}

	~resize_buffer()
	{
		if (buf_)
		{
			pool_.release(buf_);
			buf_.reset();
		}

		WISE_ENSURE(!buf_);
	}

	/// len만큼 쓴다. seg 부족하면 풀에서 할당
	std::size_t append(const uint8_t* p, std::size_t len)
	{
		WISE_EXPECT(p != nullptr);
		WISE_EXPECT(len > 0);

		WISE_RETURN_IF(p == nullptr, 0);
		WISE_RETURN_IF(len == 0, 0);

		reserve(len);

		WISE_ASSERT(buf_);
		WISE_ASSERT(buf_->capacity() >= pos_ + len);

		memcpy_s(buf_->data() + pos_, buf_->capacity() - pos_, (void*)p, len);

		pos_ += len;

		WISE_ASSERT(pos_ <= buf_->capacity());

		return len;
	}

	/// 편의 함수.
	std::size_t append(const char* p, std::size_t len)
	{
		return append((uint8_t*)p, len);
	}

	/// 편의 함수
	std::size_t append(const void* p, std::size_t len)
	{
		return append((uint8_t*)p, len);
	}

	/// false when underflow
	bool read(uint8_t* p, std::size_t len)
	{
		WISE_ASSERT(p != nullptr);
		WISE_ASSERT(len > 0);

		WISE_RETURN_IF(p == nullptr, false);
		WISE_RETURN_IF(len == 0, true);

		WISE_ASSERT(pos_ <= buf_->capacity());
		WISE_ASSERT(read_pos_ <= buf_->capacity());

		if (pos_ < (read_pos_ + len))
		{
			return false; // not available
		}

		std::memcpy((void*)p, data() + read_pos_, len);

		read_pos_ += len;

		WISE_ASSERT(pos_ <= buf_->capacity());
		WISE_ASSERT(read_pos_ <= buf_->capacity());

		return true;
	}

	/// 전체 바이트 크기 돌려줌
	std::size_t size() const
	{
		return pos_;
	}

	/// 현재 버퍼 크기 돌려줌
	std::size_t capacity() const
	{
		WISE_RETURN_IF(!buf_, 0);
		return buf_->capacity();
	}

	/// change the size of buffer. position is changed.
	void resize(const std::size_t new_size)
	{
		if (new_size <= capacity())
		{
			pos_ = new_size; // change size(). STL
			return;
		}

		reserve(new_size - capacity());

		WISE_ENSURE(buf_);

		pos_ = new_size;

		WISE_ASSERT(pos_ <= buf_->capacity());
		WISE_ASSERT(read_pos_ <= buf_->capacity());
	}

	const uint8_t* data() const
	{
		WISE_RETURN_IF(!buf_, nullptr);
		return buf_->data();
	}

	/// use this with care
	uint8_t* data()
	{
		WISE_RETURN_IF(!buf_, nullptr);
		return buf_->data();
	}

	/// get a value at pos
	const uint8_t at(std::size_t pos) const
	{
		WISE_ASSERT(buf_);
		WISE_RETURN_IF(!buf_, 0);

		WISE_ASSERT(pos < size());
		WISE_THROW_IF_FMT(pos >= size(), "index out of range. {}/{}", pos, pos_);

		return buf_->data()[pos];
	}

	/// 세그먼트 앞으로 돌린다.
	void rewind(std::size_t pos = 0)
	{
		WISE_EXPECT(size() > 0 && pos < size() || pos == 0);
		WISE_RETURN_IF(pos >= size()); // safe. no effect

		pos_ = pos;

		rewind_read();

		WISE_ASSERT(pos_ <= buf_->capacity());
		WISE_ASSERT(read_pos_ <= buf_->capacity());
	}

	void rewind_read()
	{
		read_pos_ = 0;
	}

	/// block에서 count만큼 제거 
	/**
	 * memmove를 사용하고 느릴 수 있으므로
	 * 호출이 많지 않도록 사용해야 함
	 */
	void pop_front(std::size_t count)
	{
		WISE_ASSERT(buf_);
		WISE_RETURN_IF(!buf_);

		WISE_ASSERT(count > 0);
		WISE_RETURN_IF(count == 0);

		WISE_ASSERT(count <= pos_);
		if (count > pos_)
		{
			WISE_THROW("popping more than existing bytes");
		}

		count = std::min(count, size());

		memmove_s(
			buf_->data(), buf_->capacity(),
			buf_->data() + count, pos_ - count
		);

		pos_ -= count;

		if (count <= read_pos_)
		{
			read_pos_ -= count;
		}
		else
		{
			read_pos_ = 0;
		}
	}

	iterator begin()
	{
		WISE_RETURN_IF(!buf_, 0);
		return iterator(buf_->data());
	}

	iterator end()
	{
		WISE_RETURN_IF(!buf_, 0);
		return iterator(&buf_->data()[pos_]);
	}

	const_iterator cbegin()
	{
		WISE_RETURN_IF(!buf_, 0);
		return const_iterator(buf_->data());
	}

	const_iterator cend()
	{
		WISE_RETURN_IF(!buf_, 0);
		return const_iterator(&buf_->data()[pos_]);
	}

	/// for test and debugging
	const fixed_size_buffer_pool* get_pool(std::size_t size) const
	{
		return pool_.get_pool(size);
	}

private:
	void reserve(std::size_t len)
	{
		WISE_EXPECT(len > 0);

		if (!buf_)
		{
			WISE_ASSERT(pos_ == 0);
			buf_ = pool_.alloc(std::max<std::size_t>(len, initial_size_));
		}
		else
		{
			if (buf_->capacity() < (pos_ + len))
			{
				auto nbuf = pool_.alloc(pos_ + len);

				memcpy_s(nbuf->data(), nbuf->capacity(), buf_->data(), pos_);

				pool_.release(buf_);

				buf_ = nbuf;
			}
		}

		WISE_ENSURE(buf_);
		WISE_ENSURE(capacity() >= len);
	}

private:
	static pool				pool_;
	std::size_t				initial_size_ = 1024;
	pool::buffer::ptr		buf_;
	std::size_t				pos_ = 0;
	std::size_t				read_pos_ = 0;
};

} // kernel
} // wise

