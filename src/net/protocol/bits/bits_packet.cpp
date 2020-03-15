#include <pch.hpp>
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>

namespace wise {
namespace kernel {

std::atomic<std::size_t> bits_packet::alloc_ = 0;
std::atomic<std::size_t> bits_packet::dealloc_ = 0;

bool bits_packet::pack(bits_packer& packer)
{
	WISE_UNUSED(packer);
	return true;
}

bool bits_packet::unpack(bits_packer& packer)
{
	WISE_UNUSED(packer);
	return true;
}

} // kernel
} // wise