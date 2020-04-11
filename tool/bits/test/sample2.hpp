#pragma once
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace common {
namespace detail {

enum class TestEnum
{
	V1,
	V2,
	V3 = 100,
};


struct hello
{
	uint32_t v = 1;
	int16_t iv = 3;
	std::vector<uint32_t> ids;
	TestEnum test;
};


} // common
} // detail

// struct serialization section
namespace wise { 
namespace kernel { 

// ::common::detail::hello serialization begin { 
template<> inline bool pack(bits_packer& packer, const ::common::detail::hello& tv) 
{
	packer.pack(tv.v);
	packer.pack(tv.iv);
	packer.pack(tv.ids);
	packer.pack_enum(tv.test);
	return packer.is_valid();
}

template<> inline bool unpack(bits_packer& packer, ::common::detail::hello& tv) 
{
	packer.unpack(tv.v);
	packer.unpack(tv.iv);
	packer.unpack(tv.ids);
	packer.unpack_enum(tv.test);
	return packer.is_valid();
}
// } ::common::detail::hello serialization end

} // kernel
} // wise

