#include "pch.hpp"
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/core/mem_tracker.hpp>
#include "play_message.hpp"

#define WISE_ADD_BITS(cls) wise::kernel::bits_factory::inst().add( cls::get_topic(), []() { return wise_shared<cls>(); } );

void add_play_message()
{
	WISE_ADD_BITS(::real::syn_enter_player);
	WISE_ADD_BITS(::real::syn_leave_player);
	WISE_ADD_BITS(::real::syn_begin_ready);
	WISE_ADD_BITS(::real::req_player_ready);
	WISE_ADD_BITS(::real::res_player_ready);
	WISE_ADD_BITS(::real::syn_start_play);
	WISE_ADD_BITS(::real::req_input_rsp);
	WISE_ADD_BITS(::real::syn_rsp_result);
	WISE_ADD_BITS(::real::syn_game_end);
	WISE_ADD_BITS(::real::syn_game_invalid_state);
}
