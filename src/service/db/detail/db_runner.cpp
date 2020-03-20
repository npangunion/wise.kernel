#include "stdafx.h"
#include "db_runner.hpp"
#include <wise/service/db/db_service.hpp>

namespace wise
{

db_runner::db_runner(db_service& _service)
	: service_(_service)
{
}

db_runner::~db_runner()
{
	finish();
}

bool db_runner::start(const config& cfg)
{
	WISE_ASSERT(stop_);

	config_ = cfg;

	auto rc = pool_.start(config_.dbs);

	if (!rc)
	{
		return false;
	}

	stop_ = false;

	WISE_INFO("db_runner starting...");

	std::thread([this]() { run(); }).swap(thread_);

	return true;
}

void db_runner::schedule(tx::ptr trans)
{
	WISE_ASSERT(!stop_);

	q_.push(trans);
}

void db_runner::finish()
{
	WISE_RETURN_IF(stop_);

	stop_ = true;

	thread_.join();

	WISE_INFO("db_runner finished");
}

void db_runner::run()
{
	WISE_INFO("db_runner running...");

	simple_tick loop_timer;

	while (!stop_)
	{
		uint32_t loop_count = 0;

		loop_timer.reset();

		tx::ptr trans; 

		while (q_.pop(trans))
		{
			execute_query(trans);

			loop_count++;
		}

		after_loop(loop_count, loop_timer);
	}

	complete_queue();
}

void db_runner::after_loop(uint32_t lc, simple_tick& lt)
{
	last_loop_time_ = lt.elapsed() * 0.001f;
	total_loop_time_ += last_loop_time_;
	average_loop_time_ = (last_loop_time_ + average_loop_time_) / 2;

	if (lc == 0 || last_loop_time_ < config_.idle_check_threshold_time)
	{
		std::this_thread::sleep_for(
			std::chrono::milliseconds(config_.idle_sleep_time_ms)
		);

		++total_sleep_count_;
	}
}

void db_runner::execute_query(tx::ptr trans)
{
	auto cmd = pool_.get(
		trans->get_context().db, 
		trans->get_topic(), 
		trans->get_query()
	);

	if (cmd)
	{
		auto stick = service_.get_tick();
		
		// command에서 응답 처리
		(void)cmd->execute(trans);

		auto etick = service_.get_tick();
		auto elapsed = (etick - stick);

		trans->get_context().query_time = elapsed;

		if (elapsed >= config_.slow_query_time)
		{
			WISE_WARN(L"slow query: {}, time: {}ms", trans->get_query(), elapsed);
		}
	}
	else
	{
		WISE_ERROR(
			"db runner failed to get command. tx: {}", 
			trans->get_desc()
		);
	}
}

void db_runner::complete_queue()
{
	WISE_INFO("db_runner completing tx...");

	tx::ptr trans;

	int cx = 0;

	while (q_.pop(trans))
	{
		execute_query(trans);

		++cx;
	}

	WISE_INFO("db_runner completed {} tx", cx);
}

} // wise