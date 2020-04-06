#include "stdafx.h"
#include "act_connect.hpp"
#include <wise/service/bot/agent.hpp>
#include <wise/server/server.hpp>
#include <wise/net/protocol/sys_messages.hpp>

namespace wise
{

act_connect::act_connect(wise::flow& owner, config& cfg)
	: act(owner, cfg)
{
	load();

	sub(sys_session_ready::get_topic(), WISE_CHANNEL_CB(on_session_ready));
	sub(sys_connect_failed::get_topic(), WISE_CHANNEL_CB(on_connect_failed));
}

void act_connect::on_begin()
{
	network::inst().connect(addr_, "zen", get_agent().get_channel().get_pkey());
}

void act_connect::on_session_ready(message::ptr mp)
{
	if (!is_active())
	{
		return; // it's not my session
	}

	auto sp = cast<sys_session_ready>(mp);

	auto sref = network::inst().acquire(sp->sid);

	if (sref)
	{
		if (addr_ == sref->get_remote_addr())
		{
			sref->bind_oid(get_agent().get_index());

			get_agent().get_dic().set(name_, sp->sid);

			sid_ = sp->sid;

			succeed(
				fmt::format("connected to remote. name: {}, addr: {}", name_, addr_)
			);
		}
	}
}

void act_connect::on_connect_failed(message::ptr mp)
{
	fail("connect failed to login");
}

void act_connect::load()
{
	auto nname = get_config()["name"];
	WISE_JSON_CHECK(!nname.is_null() && nname.is_string());

	name_ = nname.get<std::string>();
	WISE_JSON_CHECK(!name_.empty());

	auto naddr = get_config()["address"];
	WISE_JSON_CHECK(!naddr.is_null() && naddr.is_string());

	addr_ = naddr.get<std::string>();
}

} // wise