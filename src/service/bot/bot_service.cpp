#include "stdafx.h"
#include <wise/service/bot/bot_service.hpp>

#include <wise/base/exception.hpp>

#include <wise/service/bot/report.hpp>
#include <wise/service/bot/act_factory.hpp>
#include <wise/service/bot/acts/act_dummy.hpp>
#include <wise/service/bot/acts/act_wait.hpp>
#include <wise/service/bot/acts/act_loop.hpp>
#include <wise/service/bot/acts/act_connect.hpp>
#include <wise/service/bot/acts/act_disconnect.hpp>

#include <wise/service/db/tx.hpp>
#include <wise/server/server.hpp>
#include <wise/server/util/json_helper.hpp>
#include <wise/net/protocol/sys_messages.hpp>
#include <wise/net/network.hpp>
#include <fstream>

namespace wise
{

bot_service::bot_service(
	server& _server, const std::string& name, const config& _config) 
	: service(_server, name, _config)
{
	set_priority(task::priority::realtime);
	add_internal_acts();
	load_config();
}

bot_service::~bot_service()
{
}

void bot_service::post_by_session(packet::ptr pkt)
{
	WISE_RETURN_IF(!pkt);

	auto zp = std::static_pointer_cast<zen_message>(pkt);
	WISE_RETURN_IF(!zp);

	auto sref = network::inst().acquire(zp->sid);

	if (sref)
	{
		post_by_agent_id(sref.get()->get_bound_oid(), pkt);
	}
}

void bot_service::post_by_tx(packet::ptr pkt)
{
	WISE_RETURN_IF(!pkt);

	auto tp = std::static_pointer_cast<tx>(pkt);
	WISE_RETURN_IF(!tp);

	post_by_agent_id(tp->get_context().oid, pkt);
}

void bot_service::post_by_agent_id(uint64_t id, packet::ptr pkt)
{
	WISE_RETURN_IF(id == 0);

	auto index = get_index_from_agent_id(id);
	WISE_RETURN_IF(!is_valid_agent_index(index));

	agents_[index]->get_channel().publish(pkt);
}

bool bot_service::on_start()
{
	auto rc = service::on_start();
	WISE_RETURN_IF(!rc, rc);

	subscribe_posts();

	get_timer().repeat(5000, [this](int timer) 
	{ 
		if (check_exit())
		{
			get_timer().cancel(timer);
		}
	});

	start_report();

	return start_agents();
}

uint32_t bot_service::get_index_from_agent_id(uint64_t id)
{
	WISE_RETURN_IF(id < begin_index_, 0);
	WISE_RETURN_IF(id > begin_index_ + agent_count_, 0);

	return static_cast<uint32_t>(id - begin_index_);
}

bool bot_service::is_valid_agent_index(uint32_t index)
{
	return index >= 0 && index < agents_.size();
}

handler::result bot_service::on_execute()
{
	auto rc = service::on_execute();
	WISE_RETURN_IF(rc != result::success, rc);

	report::inst().exec();

	log()->flush();

	return result::success;
}

void bot_service::on_finish()
{
	report::inst().stop();

	stop_agents();

	service::on_finish();
}

bool bot_service::check_exit()
{
	for (auto& ap : agents_)
	{
		if (!ap->is_finished())
		{
			return false;
		}
	}

	cancel(); // let's exit

	return true;
}

void bot_service::subscribe_posts()
{
	// ¿¬°á 
	WISE_SUBSCRIBE_REDIRECT(sys_connect_failed);
	WISE_SUBSCRIBE_REDIRECT(sys_session_ready);
	WISE_SUBSCRIBE_REDIRECT(sys_session_closed);

	for (auto& pic : tx_posts_)
	{
		sub_global_redirect(pic, WISE_CHANNEL_CB(on_tx_message));
	}

	for (auto& pic : ss_posts_)
	{
		sub_global_redirect(pic, WISE_CHANNEL_CB(on_session_message));
	}
}

void bot_service::on_sys_connect_failed(message::ptr mp)
{
	auto mc = cast<sys_connect_failed>(mp);
	auto ch = channel::find(mc->pkey);
	WISE_RETURN_IF(!ch);

	ch->publish(mp);
}

void bot_service::on_sys_session_ready(message::ptr mp)
{
	auto mc = cast<sys_session_ready>(mp);
	auto sref = WiSE_GET_SESSION(mc->sid);
	WISE_RETURN_IF(!sref);

	auto ch = channel::find(sref.get()->get_bound_channel());
	WISE_RETURN_IF(!ch);

	ch->publish(mp);
}

void bot_service::on_sys_session_closed(message::ptr mp)
{
	auto mc = cast<sys_session_closed>(mp);
	auto sref = WiSE_GET_SESSION(mc->sid);
	WISE_RETURN_IF(!sref);

	auto ch = channel::find(sref.get()->get_bound_channel());
	WISE_RETURN_IF(!ch);

	ch->publish(mp);
}

void bot_service::on_tx_message(message::ptr mp)
{
	auto tp = cast<tx>(mp);
	post_by_tx(tp);
}

void bot_service::on_session_message(message::ptr mp)
{
	auto pkt = cast<packet>(mp);
	post_by_session(pkt);
}

bool bot_service::load_config()
{
	auto& cfg = get_config();

	auto nfile = cfg["test_file"];

	std::string file = "test.json";

	if (!nfile.is_null())
	{
		file = nfile.get<std::string>();
	}

	std::ifstream fs(file);

	if (!fs.is_open())
	{
		WISE_THROW_FMT(fmt::format("File [{}] not found", file));
	}

	test_config_ = nlohmann::json::parse(fs);

	load_test_config();

	return true;
}

bool bot_service::load_test_config()
{
	auto& cfg = test_config_;

	auto nbegin = cfg["begin_index"];

	if (!nbegin.is_null())
	{
		begin_index_ = nbegin.get<uint32_t>();
	}

	auto nac = cfg["agent_count"];
	if (!nac.is_null())
	{
		agent_count_ = nac.get<uint32_t>();
	}

	begin_index_ = std::max<uint32_t>(1, begin_index_);
	agent_count_ = std::max<uint32_t>(1, agent_count_);

	load_post_config();

	return true;
}

bool bot_service::load_post_config()
{
	auto& cfg = test_config_;

	auto nposts = cfg["posts"];
	WISE_JSON_CHECK(!nposts.is_null() && nposts.is_object());

	auto nbtxs = nposts["by_tx"];
	WISE_JSON_CHECK(!nbtxs.is_null() && nbtxs.is_array());

	for (auto& nbtx : nbtxs)
	{
		auto ncat = nbtx["category"];
		WISE_JSON_CHECK(!ncat.is_null() && ncat.is_number());

		auto ngrp = nbtx["group"];
		WISE_JSON_CHECK(!ngrp.is_null() && ngrp.is_number());

		topic::category_t c = ncat.get<topic::category_t>();
		topic::group_t g = ngrp.get<topic::group_t>();

		topic pic(c, g, 0);
		auto desc = nbtx["desc"].is_null() ? "" : nbtx["desc"].get<std::string>();
		
		WISE_INFO("adding post by tx. topic: {}, desc: {}", topic::get_desc(pic), desc);

		tx_posts_.push_back(pic);
	}

	auto nbsss = nposts["by_session"];
	WISE_JSON_CHECK(!nbsss.is_null() && nbtxs.is_array());

	for (auto& nbss : nbsss)
	{
		auto ncat = nbss["category"];
		WISE_JSON_CHECK(!ncat.is_null() && ncat.is_number());

		auto ngrp = nbss["group"];
		WISE_JSON_CHECK(!ngrp.is_null() && ngrp.is_number());

		topic::category_t c = ncat.get<topic::category_t>();
		topic::group_t g = ngrp.get<topic::group_t>();

		topic pic(c, g, 0);
		auto desc = nbss["desc"].is_null() ? "" : nbss["desc"].get<std::string>();
		
		WISE_INFO("adding post by session. topic: {}, desc: {}", topic::get_desc(pic), desc);

		ss_posts_.push_back(pic);
	}

	return true;
}

bool bot_service::start_report()
{
	auto nrpt = test_config_["report"];
	std::string  rpt("report.csv");

	if (!nrpt.is_null())
	{
		rpt = nrpt.get<std::string>();
	}

	auto nenable = test_config_["enable_report"];
	bool enable_report = true;

	if (!nenable.is_null())
	{
		enable_report = nenable.get<bool>();
	}

	auto rc = report::inst().start(rpt);
	WISE_RETURN_IF(!rc, false);

	if (!enable_report)
	{
		report::inst().disable();
	}

	return true;
}

bool bot_service::start_agents()
{
	for (uint32_t i = 0; i < agent_count_; ++i)
	{
		auto ap = wise_shared<agent>(
			get_server(), begin_index_ + i, test_config_
			);
		agents_.push_back(ap);
		get_server().get_scheduler().add(ap);
	}

	return true;
}

void bot_service::stop_agents()
{
	for (auto& ap : agents_)
	{
		ap->cancel();
	}

	agents_.clear();
}

void bot_service::add_internal_acts()
{
	WISE_ADD_ACT("act_dummy", wise::act_dummy);
	WISE_ADD_ACT("act_wait", wise::act_wait);
	WISE_ADD_ACT("act_loop", wise::act_loop);
	WISE_ADD_ACT("act_connect", wise::act_connect);
	WISE_ADD_ACT("act_disconnect", wise::act_disconnect);
}

} // wise
