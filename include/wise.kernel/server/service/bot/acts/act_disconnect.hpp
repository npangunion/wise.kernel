#pragma once

#include <wise/service/bot/act.hpp>

namespace wise
{

class act_disconnect : public wise::act
{
public:
	act_disconnect(wise::flow& owner, config& cfg);

private:
	void on_begin() override;

	void load();

private:
	std::string name_;
};

} // wise

