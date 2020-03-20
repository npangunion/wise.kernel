#include "stdafx.h"
#include "act_wait.hpp"
#include <wise/service/bot/agent.hpp>

namespace wise
{

act_wait::act_wait(flow& owner, config& cfg)
	: act(owner, cfg)
{
	load();

	WISE_ASSERT(wait_ > 0);
}

void act_wait::on_begin()
{
	get_agent().get_timer().once(wait_, [this](int timer)
	{
		WISE_UNUSED(timer);
		trigger("success");
	});
}

void act_wait::load()
{
	auto ntime = get_config()["time"];
	auto nrand = get_config()["random"];

	wait_ = ntime.is_null() ? 1000 : ntime.get<uint32_t>();
	wait_ = std::max<uint32_t>(1, wait_);

	if (!nrand.is_null())
	{
		auto is_random = nrand.get<bool>();

		if (is_random)
		{
			wait_ = (std::rand() % wait_);
		}
	}

	WISE_DEBUG("act_wait. time: {}", wait_);
}

} // test 
