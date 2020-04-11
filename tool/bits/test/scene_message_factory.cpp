#include "pch.hpp"
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/core/mem_tracker.hpp>
#include "scene_message.hpp"

#define WISE_ADD_BITS(cls) wise::kernel::bits_factory::inst().add( cls::get_topic(), []() { return wise_shared<cls>(); } );

void add_scene_message()
{
	WISE_ADD_BITS(::real::req_enter_scene);
	WISE_ADD_BITS(::real::res_enter_scene);
	WISE_ADD_BITS(::real::req_reenter_scene);
	WISE_ADD_BITS(::real::res_reenter_scene);
	WISE_ADD_BITS(::real::req_leave_scene);
	WISE_ADD_BITS(::real::res_leave_scene);
	WISE_ADD_BITS(::real::syn_master_scene_handler);
	WISE_ADD_BITS(::real::req_ss_create_scene);
	WISE_ADD_BITS(::real::res_ss_create_scene);
	WISE_ADD_BITS(::real::req_ss_enter_scene);
	WISE_ADD_BITS(::real::res_ss_enter_scene);
	WISE_ADD_BITS(::real::req_ss_leave_scene);
	WISE_ADD_BITS(::real::res_ss_leave_scene);
	WISE_ADD_BITS(::real::req_ss_dispose_scene);
	WISE_ADD_BITS(::real::res_ss_dispose_scene);
}
