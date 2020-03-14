#pragma once
#include <wise/net/protocol/zen/zen_message.hpp>
#include <wise/net/protocol/zen/zen_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace common {

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

// struct serialization section
namespace wise {

// ::common::hello serialization begin { 
template<> inline bool pack(zen_packer& packer, const ::common::hello& tv) 
{
	packer.pack(tv.v);
	packer.pack(tv.iv);
	packer.pack(tv.ids);
	packer.pack_enum(tv.test);
	return packer.is_valid();
}

template<> inline bool unpack(zen_packer& packer, ::common::hello& tv) 
{
	packer.unpack(tv.v);
	packer.unpack(tv.iv);
	packer.unpack(tv.ids);
	packer.unpack_enum(tv.test);
	return packer.is_valid();
}
// } ::common::hello serialization end

} // wise

