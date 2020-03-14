#include "stdafx.h"
#include "zen_message.hpp"

namespace wise
{

std::atomic<std::size_t> zen_message::alloc_ = 0;
std::atomic<std::size_t> zen_message::dealloc_ = 0;

bool zen_message::pack(zen_packer& packer)
{
	WISE_UNUSED(packer);
	return true;
}

bool zen_message::unpack(zen_packer& packer)
{
	WISE_UNUSED(packer);
	return true;
}

} // wise