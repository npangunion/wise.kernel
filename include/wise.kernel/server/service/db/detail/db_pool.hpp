#pragma once

#include "db.hpp"
#include "db_command.hpp"
#include <wise/dbc/dbc.hpp>

namespace wise
{

/// per runner db pool of connection and command
class db_pool 
{
public: 
	/// constructor
	db_pool(); 
	
	/// setup by connecting to each db. throws wise::exception if fail
	bool start(const std::vector<db>& dbs);

	/// cleanup 
	~db_pool();

	/// returns command. create one if not exists.
	db_command::ptr get(const std::string& db, const topic& pic, const std::wstring& query);

	/// to test and debug 
	bool has_db(const std::string& db) const;

	/// to test and debug
	bool has_command(const std::string& db, const topic& pic) const;

private:
	struct cmd_key
	{
		std::string db; 
		topic pic;

		/// comparison operator for map
		bool operator < (const cmd_key& rhs) const
		{
			return db < rhs.db && pic < rhs.pic;
		}

		bool operator == (const cmd_key& rhs) const
		{
			return db == rhs.db && pic == rhs.pic;
		}
	};

	using db_map = std::map<std::string, db>;
	using cmd_map = std::map<cmd_key, db_command::ptr>;


private:
	bool setup_pool(const std::vector<db>& dbs);

private:
	db_map dbs_;
	cmd_map cmds_;
};

} // wise