#pragma once 

#include <wise.kernel/core/concurrent_queue.hpp>
#include <wise.kernel/core/exception.hpp>
#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/core/mem_tracker.hpp>

#include <atomic>
#include <map>
#include <memory>
#include <vector>

namespace wise {
namespace kernel {

class fixed_size_buffer_pool
{
public:
	class buffer
	{
	public:
		using ptr = std::shared_ptr<buffer>;

		buffer(uint8_t* data, std::size_t len)
			: data_(data)
			, capacity_(len)
		{
			WISE_EXPECT(data_);
			WISE_EXPECT(capacity_ > 0);

		}

		buffer(fixed_size_buffer_pool* pool, uint8_t* data, std::size_t len)
			: pool_(pool)
			, data_(data)
			, capacity_(len)
		{
			WISE_EXPECT(pool_);
			WISE_EXPECT(data_);
			WISE_EXPECT(capacity_ > 0);
		}

		~buffer()
		{
			delete[] data_;
		}

		uint8_t* data()
		{
			return data_;
		}

		const uint8_t* data() const
		{
			return data_;
		}

		std::size_t capacity() const
		{
			return capacity_;
		}

		void mark_allocated()
		{
			WISE_ENSURE(state_ == 0x00 || state_ == 0x02);
			state_ = 0x01;
		}

		void mark_released()
		{
			WISE_ENSURE(state_ == 0x01);
			state_ = 0x02;
		}

		bool is_released() const
		{
			return state_ == 0x02;
		}

		bool is_allocated() const
		{
			return state_ == 0x01;
		}

		fixed_size_buffer_pool* get_pool() const
		{
			return pool_;
		}

	private:
		fixed_size_buffer_pool*		pool_ = nullptr;
		uint8_t*					data_ = nullptr;
		std::size_t					capacity_ = 0;
		uint8_t						state_ = 0; // 0 : create, 1 : allocated, 2 : released
	};

	struct stat
	{
		std::atomic<uint32_t> alloc_count = 0;
		std::atomic<uint32_t> os_alloc_count = 0;
		std::atomic<uint32_t> total_alloc_count = 0;
		std::atomic<uint32_t> total_release_count = 0;
	};

public:
	explicit fixed_size_buffer_pool(std::size_t length)
		: length_(length)
	{
	}

	~fixed_size_buffer_pool()
	{
	}

	std::size_t get_length() const
	{
		return length_;
	}

	std::size_t get_size() const
	{
		return blocks_.unsafe_size();
	}

	buffer::ptr alloc()
	{
		buffer::ptr block;

		++stat_.alloc_count;
		++stat_.total_alloc_count;

		if (blocks_.pop(block))
		{
			block->mark_allocated();
			return block;
		}

		++stat_.os_alloc_count;

		auto bp = wise_shared<buffer>(this, new uint8_t[get_length()], get_length());
		bp->mark_allocated();
		return bp;
	}

	void release(buffer::ptr& block)
	{
		WISE_RETURN_IF(!block->data());
		WISE_RETURN_IF(block->capacity() != length_);

		if ( !block->is_allocated() )
		{ 
			WISE_THROW("block is not allocated.");
		}

		--stat_.alloc_count;
		++stat_.total_release_count;

		block->mark_released();
		blocks_.push(block);

		WISE_ENSURE(stat_.total_alloc_count >= stat_.total_release_count);
	}

	const stat& get_stat() const
	{
		return stat_;
	}

private:
	concurrent_queue<buffer::ptr> blocks_;
	std::size_t length_ = 0;
	stat stat_;
};

} // kernel
} // wise

