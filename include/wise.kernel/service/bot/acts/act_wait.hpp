#pragma once

#include <wise/service/bot/act.hpp>

namespace wise
{

/// time: millisecs, random: bool �� ����. ���� �ð� ����� ����
class act_wait : public act
{
public:
	act_wait(flow& owner, config& cfg);

private: 
	virtual void on_begin() override;

	void load();

private: 
	uint32_t wait_ = 0;
};

} // test