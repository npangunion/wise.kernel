#pragma once 

#include <wise.kernel/net/buffer/fixed_size_buffer_pool.hpp>
#include <wise.kernel/core/mem_tracker.hpp>

#include <atomic>
#include <mutex>
#include <memory>
#include <vector>

namespace wise {
namespace kernel {

/// multiple size buffer pool
/**
 * 여러 고정 길이 버퍼를 갖고, 다양한 길이의 버퍼를 제공한다. 
 */
class multiple_size_buffer_pool
{
public:
	using buffer = fixed_size_buffer_pool::buffer;

	struct config
	{
		std::size_t start_power_of_2 = 6;	// 64 bytes 
		std::size_t	steps = 16;				// 2^24 = 1MB   
	};

public:
	multiple_size_buffer_pool()
		: config_()
	{
		WISE_EXPECT(config_.steps > 0);
		WISE_EXPECT(config_.steps < 32);
		WISE_EXPECT(config_.start_power_of_2 > 0);

		init_pools();
	}

	~multiple_size_buffer_pool()
	{
		clear();
	}

	buffer::ptr alloc(std::size_t required_size)
	{
		for (std::size_t i = 0; i < pools_.size(); ++i)
		{
			if (pools_[i]->get_length() >= required_size)
			{
				return pools_[i]->alloc();
			}
		}

		return wise_shared<buffer>(new uint8_t[required_size], required_size);
	}

	void release(buffer::ptr block)
	{
		WISE_THROW_IF(!block, "block. null pointer");
		WISE_THROW_IF(!block->data(), "block data empty");

		if (block->get_pool() == nullptr)
		{
			block.reset(); // return to os
		}
		else
		{
			block->get_pool()->release(block);
		}
	}

	void clear()
	{
		pools_.clear();
	}

	const fixed_size_buffer_pool* get_pool(std::size_t size) const
	{
		for (std::size_t i = 0; i < pools_.size(); ++i)
		{
			if (pools_[i]->get_length() == size)
			{
				return pools_[i].get();
			}
		}

		return nullptr;
	}

	std::size_t get_max_size() const
	{
		return max_size_;
	}

	void dump_stat() const;

private:
	void init_pools()
	{
		auto size = 1 << config_.start_power_of_2;

		for (std::size_t step = 0; step < config_.steps; ++step)
		{
			pools_.push_back(wise_unique<fixed_size_buffer_pool>(size));

			size <<= 1;
			max_size_ = size;
		}
	}

private:
	using pools = std::vector<std::unique_ptr<fixed_size_buffer_pool>>;

	config		config_;
	pools		pools_;
	std::size_t max_size_;
};

} // kernel
} // wise

