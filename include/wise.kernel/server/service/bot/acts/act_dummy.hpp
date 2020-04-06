#pragma once 

#include <wise/service/bot/act.hpp>

namespace wise
{

/// 1�� Ÿ�̸� �����ϰ� �����Ѵ�. �׽�Ʈ �����ӿ�ũ �׽�Ʈ��.
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