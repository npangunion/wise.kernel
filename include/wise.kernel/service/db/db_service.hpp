#pragma once 

#include <wise/server/server.hpp>
#include <wise/service/db/detail/db.hpp>
#include <wise/service/db/tx.hpp>

namespace wise
{

class db_runner;


/// db service
class db_service : public service
{
public: 
	using wid_t = uint16_t; 

public: 
	/// creates a db_service with config 
	/** 
	 * options: 
	 *  bool is_password_encrypted
	 *  int  runner_count
	 *  
	 * txs: 
	 *  uint8_t category
	 *  uint8_t group
	 *  string  desc (as a comment)
	 *
	 * db_pool with dbs: 
	 * 	std::string key;	// db key
	 *	std::string host;
	 *	std::string name; 
	 *	std::string user; 
	 *	std::string password;
	 *	std::string database;
	 */
	db_service(server& _server, const std::string& name, const config& _config);

	~db_service();

private: 
	using runner_ptr = std::unique_ptr<db_runner>;

	struct tx_sub
	{
		topic pic; 
		std::string desc;
	};

	/// 핸들러 등록. config 해석
	bool on_start() override;

	/// 인증 타임아웃 처리 
	result on_execute() override;

	/// 종료   
	void on_finish() override;

	/// process subscribed tx messages
	void on_tx(message::ptr m);

	/// subscribe to db queries (tx)
	void subscribe();

	/// start runners
	bool start_runners();

	/// replay with fail
	void fail(tx::ptr tp);

	/// load db_service configuration
	bool load_config();

private: 
	int runner_count_ = 0;
	std::vector<tx_sub> subs_;
	std::vector<db> dbs_;
	std::vector<runner_ptr> runners_;
};

} // wise