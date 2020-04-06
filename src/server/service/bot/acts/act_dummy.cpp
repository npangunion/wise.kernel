#include "stdafx.h"
#include <wise/service/bot/acts/act_dummy.hpp>
#include <wise/service/bot/agent.hpp>

namespace wise
{

act_dummy::act_dummy(flow& owner, config& cfg)
	: act(owner, cfg)
{
}

void act_dummy::on_begin()
{
	WISE_INFO("{} begins.", get_key());

	get_agent().get_timer().once( 1000, [this](int timer) 
	{ 
		WISE_UNUSED(timer);
		trigger("success"); 
	});

	++active_count_;
}

void act_dummy::on_exec()
{
	if (active_count_ > 5)
	{
		trigger("abort");
	}
}

void act_dummy::on_end()
{
	WISE_INFO("{} ends.", get_key());
}

} // wise