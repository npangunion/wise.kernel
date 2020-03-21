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
	// ��� ���... ���߿� �� ���� �� �ʿ��� �� �۾�
	// ��Ʈ��ũ�� ������ ������ �����̹Ƿ� ũ�� �ǹ� ���� �� �ִ�. 
}

void act_wait_input::load()
{
	auto ninput = get_config()["input"];
	WISE_JSON_CHECK(!ninput.is_null() && ninput.is_string());

	input_ = ninput.get<std::string>();

	WISE_DEBUG("act_wait_input. input: {}", input_);
}

} // test 
