#include "stdafx.h"
#include "act_wait_input.hpp"
#include <wise/service/bot/agent.hpp>

namespace wise
{

act_wait_input::act_wait_input(flow& owner, config& cfg)
	: act(owner, cfg)
{
	load();
}

void act_wait_input::on_begin()
{
	// 잠시 대기... 나중에 더 보고 꼭 필요할 때 작업
	// 네트워크로 진행을 제어할 예정이므로 크게 의미 없을 수 있다. 
}

void act_wait_input::load()
{
	auto ninput = get_config()["input"];
	WISE_JSON_CHECK(!ninput.is_null() && ninput.is_string());

	input_ = ninput.get<std::string>();

	WISE_DEBUG("act_wait_input. input: {}", input_);
}

} // test 
