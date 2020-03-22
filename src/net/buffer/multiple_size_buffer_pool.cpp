#include <pch.hpp>
#include <wise.kernel/net/buffer/multiple_size_buffer_pool.hpp>

namespace wise {
namespace kernel {

void multiple_size_buffer_pool::dump_stat() const
{
	for (std::size_t i = 0; i < pools_.size(); ++i)
	{
		auto& stat = pools_[i]->get_stat();

		WISE_INFO("mutiple_size_buffer_pool. size:{}, total alloc:{}, os_alloc:{}, total release:{}",
			pools_[i]->get_length(),
			stat.total_alloc_count,
			stat.os_alloc_count,
			stat.total_release_count);
	}
}

} // kernel
} // wise