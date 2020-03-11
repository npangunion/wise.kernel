#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/base/timer.hpp>
#include <chrono>
#include <thread>

using namespace wise::kernel;
using namespace std::literals::chrono_literals;

TEST_CASE("timer", "base")
{
	SECTION("usage")
	{
		timer tm;

		int id = tm.set(1);

		int count = 0;

		tm.add(id, [&count](int tid) {
			WISE_UNUSED(tid);
			++count;
			});

		std::this_thread::sleep_for(10ms);
		tm.execute();

		std::this_thread::sleep_for(10ms);
		tm.execute();

		std::this_thread::sleep_for(10ms);
		tm.execute();

		REQUIRE(count == 3);
	}
}