#pragma once

#include <wise/service/bot/act.hpp>

namespace wise
{

/// time: millisecs, random: bool 로 지정. 일정 시간 대기후 성공
class act_loop : public act
{
public:
	act_loop(flow& owner, config& cfg);

private: 
	void on_begin() override;

	void on_exec() override;

	void load();

private: 
	uint32_t loop_ = 1;
	uint32_t current_loop_ = 0;
};

} // test