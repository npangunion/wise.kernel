#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/task/task.hpp>
#include <wise.kernel/util/util.hpp>

#pragma warning(disable: 6319)

using namespace wise::kernel;

class my_task : public task
{
public: 
	my_task() {}
	
private:
	result on_execute() override
	{
		sleep(10);
		return result::success;
	}
};

TEST_CASE("task", "core")
{

	SECTION("usage")
	{
		my_task mt;

		CHECK(mt.start());

		CHECK(mt.execute(1) == task::result::success);
		CHECK(mt.get_last_execution_time() >= 10);
		CHECK(mt.get_execution_count() == 1);

		mt.pause();

		CHECK(mt.execute(1) == task::result::success);
		CHECK(mt.get_last_execution_time() <= 1);
		CHECK(mt.get_execution_count() == 1);

		mt.resume();

		CHECK(mt.execute(1) == task::result::success);
		CHECK(mt.get_last_execution_time() >= 10);
		CHECK(mt.get_execution_count() == 2);

		mt.finish();

		CHECK(mt.execute(1) == task::result::finished);
	}
}