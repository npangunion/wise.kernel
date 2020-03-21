#include "stdafx.h"
#include <wise/service/db/db_service.hpp>
#include <wise/service/db/tx.hpp>
#include <wise/service/db/detail/db_runner.hpp>
#include <wise/net/network.hpp>
#include <wise/dbc/convert.hpp>
#include <wise/server/util/json_helper.hpp>
#include <wise/base/memory.hpp>

constexpr int default_runner_count = 16;
constexpr int min_runner_count = 1;

namespace wise
{

db_service::db_service(server& _server, const std::string& name, const config& _config)
	: service(_server, name, _config)
{
}

db_service::~db_service()
{
}

bool db_service::on_start()
{
	auto rc = service::on_start();
	WISE_RETURN_IF(!rc, false);

	rc = load_config();
	WISE_RETURN_IF(!rc, false);

	subscribe();

	return start_runners();
}

handler::result db_service::on_execute()
{
	auto rc = service::on_execute();
	WISE_RETURN_IF(rc != handler::result::success, rc);

	return handler::result::success;
}

void db_service::on_finish()
{
	for (auto& r : runners_)
	{
		r->finish();
	}

	runners_.clear();

	service::on_finish();
}

void db_service::on_tx(message::ptr m)
{
	auto tran = std::static_pointer_cast<tx>(m);
	WISE_ASSERT(tran);

	auto& ctx = tran->get_context();

	if (ctx.dir == 1)
	{
		// db_service process requests only
		return;
	}

	if (runners_.empty())
	{
		ctx.rc = tx::result::fail_runner_not_available;
		ctx.dir = 1;

		fail(tran);

		return;
	}

	auto default_index = rand() % runners_.size();
	auto target = runners_[default_index].get();
	
	if (ctx.key > 0)
	{
		auto key_index = ctx.key % runners_.size();
		target = runners_[key_index].get();
	}

	target->schedule(tran);
}

void db_service::subscribe()
{
	for (auto& ts : subs_)
	{
		WISE_SUBSCRIBE_REDIRECT_DIRECT(ts.pic, on_tx);
	}
}

bool db_service::start_runners()
{
	runners_.resize(runner_count_);

	for (int i = 0; i < runner_count_; ++i)
	{
		auto ptr = wise_unique<db_runner>(*this);
		runners_[i].swap(ptr);
	}

	db_runner::config drc; 
	drc.dbs = dbs_;

	for (int i=0; i<runner_count_; ++i)
	{
		auto rc = runners_[i]->start(drc);

		if (!rc)
		{
			WISE_ERROR("failed to start db_runner {}", i);
			return false;
		}
	}

	return true;
}

void db_service::fail(tx::ptr tran)
{
	if (tran->sid > 0)
	{
		auto ss = network::inst().acquire(tran->sid);

		// remote send
		ss.send(tran);
	}
	else
	{
		// local post
		network::post(tran);
	}
}

bool db_service::load_config()
{
	/**
	* options:
	*  bool is_password_encrypted
	*  int  runner_count
	*
	* tx:
	*  uint8_t category
	*  uint8_t group
	*  string  desc (as a comment)
	*/

	auto& cfg = get_config();

	try
	{
		auto ns = cfg["runner_count"];

		if (ns.is_null())
		{
			runner_count_ = default_runner_count;
		}
		else
		{
			runner_count_ = std::max(ns.get<int>(), min_runner_count);
		}

		// tx : [ { "category" : 3, "group" : 5, "desc" : "auth transactions" }, ... ] 	

		auto ntx = cfg["txs"];
		WISE_JSON_CHECK(!ntx.is_null() && ntx.is_array());

		for (auto& nx : ntx)
		{
			if (nx["category"].is_null())
			{
				WISE_ERROR("tx must have category. json: {}", nx.dump());
				return false;
			}

			if (nx["group"].is_null())
			{
				WISE_ERROR("tx must have group. json: {}", nx.dump());
				return false;
			}

			topic::category_t c = nx["category"].get<topic::category_t>();
			topic::group_t g = nx["group"].get<topic::group_t>();

			topic pic(c, g, 0);
			auto desc = nx["desc"].is_null() ? "" : nx["desc"].get<std::string>();

			tx_sub sub{ pic, desc};

			subs_.push_back(sub);

			WISE_INFO("adding tx to process. topic: {}, desc: {}", topic::get_desc(pic), desc);
		}

	   /*
		* db_pool with dbs:
		* 	std::string key;	// db key
		*	std::string host;
		*	std::string name;
		*	std::string user;
		*	std::string password;
		*	std::string database;
		*/

		auto ndb = cfg["dbs"];

		for (auto& nd : ndb)
		{
			if (nd["key"].is_null())
			{
				WISE_ERROR("db must have a key. json: {}", nd.dump());
				return false;
			}

			if (nd["host"].is_null())
			{
				WISE_ERROR("db must have a host. json: {}", nd.dump());
				return false;
			}

			if (nd["name"].is_null())
			{
				WISE_ERROR("db must have a name. json: {}", nd.dump());
				return false;
			}

			if (nd["user"].is_null())
			{
				WISE_ERROR("db must have a user. json: {}", nd.dump());
				return false;
			}

			if (nd["password"].is_null())
			{
				WISE_ERROR("db must have a password. json: {}", nd.dump());
				return false;
			}

			if (nd["database"].is_null())
			{
				WISE_ERROR("db must have a database. json: {}", nd.dump());
				return false;
			}


			db::config dc; 

			convert(nd["key"].get<std::string>(), dc.key);
			convert(nd["host"].get<std::string>(), dc.host);
			convert(nd["name"].get<std::string>(), dc.name);
			convert(nd["user"].get<std::string>(), dc.user);
			convert(nd["password"].get<std::string>(), dc.password);
			convert(nd["database"].get<std::string>(), dc.database);

			dbs_.push_back(db(dc));

			WISE_INFO(L"adding db. host:{} name:{}", dc.host, dc.name);
		}

		return true;
	}
	catch (nlohmann::json::exception& ex)
	{
		WISE_ERROR("exception: {}", ex.what());

		return false;
	}
}

} // wise