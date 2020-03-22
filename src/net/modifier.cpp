#include <pch.hpp>
#include <wise.kernel/net/modifier.hpp>

namespace wise {
namespace kernel {

void modifier::update_length_field(
	resize_buffer& buf,
	std::size_t msg_offset,
	std::size_t new_len
)
{
	auto iter = buf.begin() + (ptrdiff_t)msg_offset;
	iter; *iter = new_len & 0x000000FF;
	++iter; *iter = new_len >> 8 & 0x000000FF;
	++iter; *iter = new_len >> 8 & 0x000000FF;
	++iter; *iter = new_len >> 8 & 0x000000FF;
}

} //kernel
} // wise
