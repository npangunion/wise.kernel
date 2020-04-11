#pragma once
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace game {

enum class user
{
	category = 201,
	group = 1,
	req_match = 1,
	res_match,
	syn_match_state,
};


enum class match_master
{
	category = 201,
	group = 2,
	syn_master_match_handler = 1,
	req_master_match,
	res_master_match,
	req_master_cancel_match,
	res_master_cancel_match,
	req_master_create_scene,
	res_master_create_scene,
	syn_master_match_state,
};


enum class query
{
	category = 201,
	group = 3,
	load_game = 1,
	save_game,
};


enum class scene_master
{
	category = 201,
	group = 4,
	syn_master_scene_handler = 1,
	req_ss_create_scene,
	res_ss_create_scene,
	req_ss_enter_scene,
	res_ss_enter_scene,
	req_ss_leave_scene,
	res_ss_leave_scene,
	req_ss_dispose_scene,
	res_ss_dispose_scene,
};


enum class scene_service
{
	category = 201,
	group = 5,
	req_enter_scene,
	res_enter_scene,
	req_reenter_scene,
	res_reenter_scene,
	req_leave_scene,
	res_leave_scene,
};


enum class play
{
	category = 201,
	group = 11,
	syn_enter_player = 1,
	syn_leave_player,
	syn_begin_ready,
	req_player_ready,
	res_player_ready,
	syn_start_play,
	req_input_rsp,
	syn_rsp_result,
	syn_game_end,
	syn_game_invalid_state,
};


} // game


