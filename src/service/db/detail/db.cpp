#include "stdafx.h"
#include <wise/service/db/detail/db.hpp>
#include <wise/dbc/error.hpp>
#include <wise/base/logger.hpp>
#include <wise/base/memory.hpp>
#include <boost/algorithm/string.hpp>

static constexpr int connect_timeout = 10;

namespace wise
{
db::db(const config& cfg)
	: config_(cfg)
{
	boost::algorithm::to_lower(config_.database);

	dsn_ = fmt::format(
		L"Driver={};Server={};Database={};UID={};PWD={}",
		L"{SQL Server Native Client 11.0}", 
		config_.host, config_.database, config_.user, config_.password
	);
}

db::db(const db& rhs)
{
	config_ = rhs.config_;
	dsn_ = rhs.dsn_;
	conn_ = rhs.conn_;
}

db& db::operator=(const db& rhs)
{
	WISE_ASSERT(&rhs != this);

	config_ = rhs.config_;
	dsn_ = rhs.dsn_;
	conn_ = rhs.conn_;

	return *this;
}

bool db::connect()
{
	try
	{
		if (conn_)
		{
			if (conn_->connected())
			{
				WISE_WARN(L"reconnecting db. dsn: {}", get_dsn());
			}
		}

		conn_.reset();
		conn_ = wise_shared<dbc::connection>();
		conn_->connect(get_dsn(), connect_timeout);

		return true;
	}
	catch (dbc::database_error& ex)
	{
		std::string dsn; 
		convert(get_dsn(), dsn);

		WISE_ERROR("db failed to connect. error: {} dsn: {}", ex.what(), dsn);
		return false;
	}
}

} // wise