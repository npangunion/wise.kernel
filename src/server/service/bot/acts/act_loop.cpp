#include "stdafx.h"
#include "act_loop.hpp"
#include <wise/service/bot/agent.hpp>

namespace wise
{

act_loop::act_loop(flow& owner, config& cfg)
	: act(owner, cfg)
{
	load();
}

void act_loop::on_begin()
{
	current_loop_++;

	WISE_DEBUG("loop. count: {}", current_loop_);
}

void act_loop::on_exec()
{
	if (current_loop_ >= loop_)
	{
		WISE_INFO("loop finished. loop: {}", loop_);
		trigger("finish");
	}
	else
	{
		trigger("loop");
	}
}

void act_loop::load()
{
	auto nloop = get_config()["loop"];
	loop_ = nloop.is_null() ? 1 : nloop.get<uint32_t>();
	WISE_DEBUG("act_loop. loop: {}", loop_);
}

} // test 
