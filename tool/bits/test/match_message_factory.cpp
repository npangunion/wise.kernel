#include "pch.hpp"
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/core/mem_tracker.hpp>
#include "match_message.hpp"

#define WISE_ADD_BITS(cls) wise::kernel::bits_factory::inst().add( cls::get_topic(), []() { return wise_shared<cls>(); } );

void add_match_message()
{
	WISE_ADD_BITS(::real::req_match);
	WISE_ADD_BITS(::real::res_match);
	WISE_ADD_BITS(::real::syn_match_state);
	WISE_ADD_BITS(::real::syn_master_match_handler);
	WISE_ADD_BITS(::real::req_master_match);
	WISE_ADD_BITS(::real::res_master_match);
	WISE_ADD_BITS(::real::req_master_cancel_match);
	WISE_ADD_BITS(::real::res_master_cancel_match);
	WISE_ADD_BITS(::real::req_master_create_scene);
	WISE_ADD_BITS(::real::res_master_create_scene);
	WISE_ADD_BITS(::real::syn_master_match_state);
}
