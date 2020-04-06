#include "stdafx.h"

#include "agent.hpp"
#include "report.hpp"
#include <wise/server/server.hpp>
#include <wise/net/network.hpp>
#include <wise/net/addr.hpp>
#include <wise/server/util/json_helper.hpp>
#include <spdlog/fmt/fmt.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace wise
{

agent::agent(server& _server, uint32_t index, config& cfg)
	: index_(index) 
	, active_(false)
	, cfg_(cfg)
	, subscription_handler(_server, fmt::format("agent_{}", index))
	, mt_rand_(tick_.since_epoch())
{
	load();

	set_priority(priority::realtime);
}

agent::~agent()
{
	WISE_ASSERT(get_state() == state::finished);
}

session_ref agent::get_session(const std::string& name)
{
	auto& dic = get_dic();
	auto rsid = dic.get<sid_t>(name);

	if (!rsid)
	{
		return session_ref();
	}

	return WiSE_GET_SESSION(rsid.value);
}

bool  agent::send(const std::string& name, packet::ptr mp)
{
	auto sref = get_session(name);
	if (!sref)
	{
		return false;
	}

	return !!sref->send(mp);
}

service_ref::ptr agent::get_service_ref(service_id_t id) const
{
	return get_server().get_service_monitor()->get_ref(id);
}

std::size_t agent::get_random()
{
	return mt_rand_();
}

bool agent::on_start()
{
	WISE_ASSERT(!active_);

	auto rc = subscription_handler::on_start();
	WISE_RETURN_IF(!rc, false);

	rc = flow_->start();

	active_ = rc;

	if (active_)
	{
		WISE_INFO(
			"agent: {} started. flow: {}",
			get_id(),
			flow_->get_key()
		);

		begin_ = get_tick();
	}
	else
	{
		WISE_INFO(
			"agent: {0} failed to start",
			get_id()
		);
	}

	return rc;
}

agent::result agent::on_execute()
{
	auto rc = subscription_handler::on_execute();

	if ( rc == result::success)
	{
		flow_->exec();
	}

	return rc;
}

void agent::on_finish()
{
	WISE_ASSERT(active_);

	flow_->stop();

	active_ = false;

	end_ = get_tick();

	report::inst().add(
		get_id(), 
		"agent", 
		"agent", 
		begin_, 
		end_, 
		end_ - begin_, 
		true
	);

	WISE_INFO(
		"agent: {0} stopped",
		get_id()
	);

	subscription_handler::on_finish();
}

act::ptr agent::get_act(const act::key& key) const
{
	return flow_->find(key);
}

bool agent::has_act(const act::key& key) const
{
	return flow_->has_act(key);
}

void agent::load()
{
	load_id();
	load_pw();
	load_flow();
}

void agent::load_id()
{
	// id 
	auto id_pre = cfg_["id_prefix"];

	if (!id_pre.is_null() && id_pre.is_string())
	{	
		char buf[128];
		sprintf_s(buf, "%s%d", id_pre.get<std::string>().c_str(), index_);
		id_ = buf;
	}
	else
	{
		id_ = fmt::format("bot_{}", index_);;
	}

	auto use_uuid = cfg_["use_uuid"];

	if (!use_uuid.is_null())
	{
		auto use = use_uuid.get<std::string>();

		if (use == "true" || use == "yes")
		{
			// generate id 
			auto uuid = boost::uuids::random_generator()();
			auto id = boost::lexical_cast<std::string>(uuid);

			id_.append("_");
			id_.append(id.substr(0, 23));

			WISE_DEBUG("uuid: {0}", id_);
		}
	}
}

void agent::load_pw()
{
	// pw
	auto pw_pre = cfg_["pw_prefix"];

	if (!pw_pre.is_null() && pw_pre.is_string())
	{	
		char buf[128];
		sprintf_s(buf, "%s%d", pw_pre.get<std::string>().c_str(), index_);
		pw_ = buf;
	}
	else
	{
		pw_ = fmt::format("bot_{}", index_);
	}
}

void agent::load_flow()
{
	auto nflow = cfg_["flow"];

	WISE_JSON_CHECK(!nflow.is_null() && nflow.is_object());

	auto nname = nflow["name"];
	auto nacts = nflow["acts"];

	WISE_JSON_CHECK(!nname.is_null() && nname.is_string());
	WISE_JSON_CHECK(!nacts.is_null() && nacts.is_array());

	flow_ = wise_shared<flow>(*this, nname.get<std::string>(), nacts);
}

} // wise
