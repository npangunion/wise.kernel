#pragma once 

#include <wise/service/bot/act.hpp>

namespace wise
{

/// 1초 타이머 설정하고 종료한다. 테스트 프레임워크 테스트용.
class act_dummy : public act
{
public: 
	act_dummy(flow& owner, config& cfg); 

private: 
	void on_begin() override;

	void on_exec() override; 

	void on_end() override;

private: 
	int active_count_ = 0;
};

} // wise