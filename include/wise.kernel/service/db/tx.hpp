#pragma once 

#include <wise/server/service/hx.hpp>
#include <wise/dbc/dbc.hpp>

#include <memory>
#include <string>

namespace wise
{

class tx  : public hx
{
public:
	using ptr = std::shared_ptr<tx>;
	using super = zen_message;

	enum class result
	{
		success, 
		fail, 
		fail_key_invalid, 
		fail_runner_not_available,
		fail_db_not_found, 
		fail_query_execute,
		fail_query_exception,	// query failed
		fail_db_exception,		// db or connection exception. error handling required
		fail_db_not_ready, 
		fail_channel_not_found, 
		fail_invalid_vector_size
	};

	struct context
	{
		uint8_t		dir = 0;	// 0 request. 1 receive
		std::string id;			// uuid
		std::string db;			// database selector
		uint64_t	key = 0;	// object key to select db_runner
		uint64_t	oid = 0;	// entity or agent id related to this query
		std::string channel;	// channel to forward reponse
		std::string error;		// error string
		result		rc = result::success;		// result code
		tick_t		query_time = 0;

		bool pack(zen_packer& packer) const;

		bool unpack(zen_packer& packer);

		std::string get_desc() const;
	};

public: 
	/// constructor
	tx(const topic& pic);

	/// execute db query. call sp
	virtual result execute_query(dbc::statement::ptr stmt) = 0;

	/// get context
	context& get_context()
	{
		return ctx_;
	}

	/// get context. const version
	const context& get_context() const
	{
		return ctx_;
	}

	/// pack base information
	bool pack(zen_packer& packer) override
	{
		super::pack(packer);
		return ctx_.pack(packer);
	}

	/// unpack base information
	bool unpack(zen_packer& packer) override
	{
		super::unpack(packer);
		return ctx_.unpack(packer);
	}

	/// get prebuilt query
	const std::wstring& get_query() const
	{
		return query_;
	}

protected: 
	context ctx_;
	std::wstring query_;
};

} // wise