#include "stdafx.h"
#include <wise/net/protocol/zen/zen_factory.hpp>
#include <wise/base/memory.hpp>
#include "simple_msg.hpp"

#define WISE_ADD_ZEN(cls) wise::zen_factory::inst().add( cls::get_topic(), []() { return wise_shared<cls>(); } );

void add_simple_msg()
{
	WISE_ADD_ZEN(::shop::req_buy_item);
	WISE_ADD_ZEN(::shop::hx_example_with_options);
}
