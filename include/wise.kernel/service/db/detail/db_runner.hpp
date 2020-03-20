#pragma once 

#include <wise/service/db/detail/db_pool.hpp>
#include <wise/base/concurrent_queue.hpp>

namespace wise
{

class db_service;

class db_runner final
{
public: 
	struct config
	{
		/// 기본 값 1ms. 이보다 짧게 걸리면 1ms 쉬기
		float idle_check_threshold_time = 0.001f;

		/// idle일 때 쉬는 시간
		unsigned int idle_sleep_time_ms = 1;

		/// 이 시간을 넘으면 로그를 남긴다.
		tick_t slow_query_time = 100;

		/// dbs to prepare connections
		std::vector<db> dbs;
	};

	using ptr = std::unique_ptr<db_runner>;

public: 
	db_runner(db_service& _service); 

	~db_runner(); 

	/// start db runner
	bool start(const config& cfg);

	/// schedule tx	
	void schedule(tx::ptr trans);

	/// stop. completes tx before exit
	void finish();

	/// for testing and debugging only
	const db_pool& get_pool() const
	{
		return pool_;
	}

private:
	void run();

	void after_loop(uint32_t lc, simple_tick& lt);

	void execute_query(tx::ptr trans);

	void complete_queue();

private: 
	db_service& service_;
	config config_;
	std::atomic<bool> stop_ = true;
	std::thread thread_;
	concurrent_queue<tx::ptr> q_;

	std::atomic<std::size_t> tx_run_count_ = 0;
	std::size_t total_sleep_count_ = 0;
	float last_loop_time_ = 0.f;
	float average_loop_time_ = 0.f;	// moving average
	float total_loop_time_ = 0.f;

	db_pool pool_;
};

} // wise