#pragma once
#include <wise/net/protocol/zen/zen_message.hpp>
#include <wise/net/protocol/zen/zen_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace db {

enum class query
{
	category = 17,
	group = 1,
	simple = 1,
};


} // db


