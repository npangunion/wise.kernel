#include "stdafx.h"
#include "act_disconnect.hpp"
#include <wise/service/bot/agent.hpp>
#include <wise/server/server.hpp>

namespace wise
{

act_disconnect::act_disconnect(wise::flow& owner, config& cfg)
	: act(owner, cfg)
{
	load();
}

void act_disconnect::on_begin()
{
	auto sref = get_agent().get_session(name_);

	sref.close();

	succeed("session close requested");
}

void act_disconnect::load()
{
	auto nname = get_config()["name"];
	WISE_JSON_CHECK(!nname.is_null() && nname.is_string());

	name_ = nname.get<std::string>();
	WISE_JSON_CHECK(!name_.empty());
}

} // wise