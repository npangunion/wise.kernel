#include "stdafx.h"
#include "db_pool.hpp"
#include <wise/base/exception.hpp>
#include <wise/base/memory.hpp>

namespace wise
{

db_pool::db_pool()
{
}

db_pool::~db_pool()
{
	cmds_.clear(); 
	dbs_.clear();
}

bool db_pool::start(const std::vector<db>& dbs)
{
	return setup_pool(dbs);
}

db_command::ptr db_pool::get(const std::string& db, const topic& pic, const std::wstring& query)
{
	auto key = cmd_key{ db, pic };

	auto ci = cmds_.find(key);

	if (ci != cmds_.end())
	{
		return ci->second;
	}

	auto di = dbs_.find(db);

	if (di == dbs_.end())
	{
		WISE_ERROR("db not found. db: {}", db);
		return db_command::ptr();
	}

	try
	{
		auto cmd = wise_shared<db_command>(
			pic,
			di->second.get_connection(),
			query);

		cmds_.insert(cmd_map::value_type(key, cmd));

		std::wstring wdb; 
		convert(db, wdb);

		WISE_INFO(L"db_command created. db: {}, query: {}", wdb, query);

		return cmd;
	}
	catch (dbc::database_error& ex)
	{
		WISE_ERROR("database error. {}", ex.what());
		return db_command::ptr();
	}
}

bool db_pool::has_db(const std::string& db) const
{
	auto di = dbs_.find(db);

	return di != dbs_.end();
}

bool db_pool::has_command(const std::string& db, const topic& pic) const
{
	auto key = cmd_key{ db, pic };

	auto ci = cmds_.find(key);
	
	return ci != cmds_.end();
}

bool db_pool::setup_pool(const std::vector<db>& dbs)
{
	WISE_ASSERT(dbs_.empty());

	for (auto& d : dbs)
	{
		db nd = d; // copy

		auto& key = nd.get_config().key;

		auto rc = nd.connect();

		if (!rc)
		{
			WISE_ERROR("db {} failed to connect. check log!", key);

			return false;
		}

		dbs_.insert(db_map::value_type(key, nd));

		WISE_INFO("db {} ready!", key);
	}

	return true;
}

} // wise