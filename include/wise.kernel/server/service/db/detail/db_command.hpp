#pragma once 

#include <wise/service/db/tx.hpp>
#include <wise/dbc/connection.hpp>
#include <wise/dbc/statement.hpp>
#include <wise/channel/message.hpp>
#include <wise/base/tick.hpp>

#include <memory>

namespace wise
{

class db_command final
{
public: 
	using ptr = std::shared_ptr<db_command>;

	struct stat
	{
		tick_t last_execution_time = 0;
		tick_t last_queue_wait_time = 0;
		tick_t total_execution_time = 0;
		std::size_t total_execution_count = 0;
	};

public: 
	/// construct. throws exception when fail
	db_command(const topic& pic, dbc::connection::ptr conn, const std::wstring& query);

	/// cleanup
	~db_command();

	/// try to reopen with a new connection. throws exception
	void reopen(dbc::connection::ptr conn);

	/// execute tx and send result back 
	tx::result execute(tx::ptr tran);

	/// check whether statement is open and prepared
	bool is_ready() const
	{
		return ready_;
	}

	/// get topic for the query
	const topic& get_topic() const
	{
		return pic_;
	}

private: 
	void open_and_preapre_stmt();
	
	tx::result post_result(tx::result rc, tx::ptr tran, const char* error);

	const std::string& get_desc() const;

private:
	topic pic_;
	std::wstring query_;
	dbc::connection::ptr conn_;
	dbc::statement::ptr stmt_;
	bool ready_ = false;
	tx::result last_result_ = tx::result::success;
	stat stat_;
	std::string desc_;
};

} // wise