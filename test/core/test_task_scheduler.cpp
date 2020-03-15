#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/task/task_scheduler.hpp>
#include <spdlog/fmt/fmt.h>

using namespace wise::kernel;

namespace
{

class my_task : public task
{
public: 
	my_task(const std::string& desc)
		: task()
	{
		set_desc(desc);
	}

private: 
	result on_execute() override
	{
		// sleep(1);

		return result::success;
	}
};

}

TEST_CASE("task scheduler")
{

	SECTION("usage")
	{
		task_scheduler::config config;
		task_scheduler ts; 

		auto rc = ts.start(config);
		CHECK(rc);
		CHECK(ts.get_runner_count() == config.runner_count);

		auto tsk = wise_shared<my_task>("my_task for usage");

		ts.add(tsk);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		CHECK(tsk->get_last_runner_id() > 0);
		CHECK(tsk->get_last_execution_time() > 0);

		ts.finish();
	}

	SECTION("performance")
	{
		task_scheduler::config config;
		task_scheduler ts;

		config.runner_count = 8;

		auto rc = ts.start(config);
		CHECK(rc);
		CHECK(ts.get_runner_count() == config.runner_count);

		const int test_count = 1;

		for (int i = 0; i < test_count; ++i)
		{
			auto desc = fmt::format("task{}", i + 1);
			auto tsk = wise_shared<my_task>(desc);

			ts.add(tsk);
		}

		for (int i = 0; i < 10; ++i)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			// ts.log_stat();
		}

		ts.finish();

		ts.log_stat();

		// overhead를 차지하는 부분
		// - concurrent_queue::push 
		// - get_target_runner_id
		// - wise::log::get() 
		// 
	}
}