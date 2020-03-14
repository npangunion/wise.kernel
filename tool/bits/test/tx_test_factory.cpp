#include "stdafx.h"
#include <wise/net/protocol/zen/zen_factory.hpp>
#include "tx_test.hpp"

#define WISE_ADD_ZEN(cls) wise::zen_factory::inst().add( cls::get_topic(), []() { return std::make_shared<cls>(); } );

void add_tx_test()
{
	WISE_ADD_ZEN(::query::simple_query);
}
