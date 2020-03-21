#pragma once

#include <wise/service/bot/act.hpp>

namespace wise
{

/// time: millisecs, random: bool �� ����. ���� �ð� ����� ����
class act_wait_input : public act
{
public:
	act_wait_input(flow& owner, config& cfg);

private: 
	virtual void on_begin() override;

	void load();

private: 
	std::string input_;
};

} // test