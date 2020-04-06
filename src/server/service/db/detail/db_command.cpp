#include "stdafx.h"
#include "db_command.hpp"
#include <wise/dbc/error.hpp>
#include <wise/dbc/convert.hpp>
#include <wise/base/util.hpp>
#include <wise/base/memory.hpp>
#include <wise/net/network.hpp>
#include <wise/net/session.hpp>

namespace wise
{

db_command::db_command(const topic& pic, dbc::connection::ptr conn, const std::wstring& query)
	: pic_(pic)
	, conn_(conn)
	, query_(query)
{
	WISE_ASSERT(pic.is_valid());
	WISE_ASSERT(conn);
	WISE_ASSERT(!query.empty());

	std::string squery;
	wise::convert(query_, squery);

	desc_ = fmt::format(
		"{}/{}/{} {}",
		pic_.get_category(), pic_.get_group(), pic_.get_type(),
		squery
	);

	open_and_preapre_stmt();
}

db_command::~db_command()
{
	stmt_.reset();
}

void db_command::reopen(dbc::connection::ptr conn)
{
	WISE_ASSERT(conn);

	conn_ = conn;

	open_and_preapre_stmt();
}

tx::result db_command::execute(tx::ptr tran)
{
	WISE_ASSERT(tran);
	WISE_ASSERT(tran->get_context().dir == 0); // request
	WISE_ASSERT(tran->get_topic() == pic_);
	WISE_ASSERT(ready_);
	WISE_RETURN_IF(!ready_, tx::result::fail_db_not_ready);

	WISE_TRACE(L"execute: {}", tran->get_query());

	try
	{
		auto rc = tran->execute_query(stmt_);

		if (rc != tx::result::success)
		{
			return post_result(
				tx::result::fail_query_execute, 
				tran, 
				"Query execute error"
			);
		}

		return post_result(tx::result::success, tran, "succss");
	}
	catch (dbc::type_incompatible_error& ex)
	{
		return post_result(tx::result::fail_query_exception, tran, ex.what());
	}
	catch (dbc::null_access_error& ex)
	{
		return post_result(tx::result::fail_query_exception, tran, ex.what());
	}
	catch (dbc::index_range_error& ex)
	{
		return post_result(tx::result::fail_query_exception, tran, ex.what());
	}
	catch (dbc::programming_error& ex)
	{
		return post_result(tx::result::fail_query_exception, tran, ex.what());
	}
	catch (dbc::database_error& ex)
	{
		return post_result(tx::result::fail_db_exception, tran, ex.what());
	}
}

void db_command::open_and_preapre_stmt()
{
	ready_ = false; 

	stmt_.reset();

	stmt_ = wise_shared<dbc::statement>();
	stmt_->open(conn_.get());
	stmt_->prepare(query_);

	ready_ = true;
}

tx::result db_command::post_result(tx::result rc, tx::ptr tran, const char* error)
{
	if (rc != tx::result::success)
	{
		WISE_ERROR("query: {}, error: {}", get_desc(), error);
	}

	auto& ctx = tran->get_context();

	ctx.dir = 1;
	ctx.error = error;
	ctx.rc = rc;

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

	return rc;
}

const std::string& db_command::get_desc() const
{
	return desc_;
}

} // wise