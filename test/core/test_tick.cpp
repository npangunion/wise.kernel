#include "pch.hpp"
#include <catch.hpp>
#include <wise.kernel/core/tick.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

TEST_CASE("tick")
{

	SECTION("fine tick")
	{
		wise::kernel::fine_tick tick;

		CHECK(tick.elapsed() < 1);

		std::this_thread::sleep_for(110ms); // 타이밍에 따라 오차가 좀 있다.

		CHECK(tick.elapsed() >= 100);

		//
		// QueryPerformanceFrequency / QueryPerformanceCounter 기반
	}

	SECTION("simple tick")
	{
		wise::kernel::simple_tick tick;

		CHECK(tick.elapsed() < 1);

		std::this_thread::sleep_for(110ms); // 타이밍에 따라 오차가 좀 있다.

		CHECK(tick.elapsed() >= 100);

		CHECK(tick.check_timeout(100, false) == true);
		CHECK(tick.check_timeout(100, true) == true);
		CHECK(tick.check_timeout(100) == false);

		// 아래 패턴으로 사용한다. 
		// if ( tick.check_timeout(100) )
		// {
		//		// do something
		// }

		// 틱 카운트 기반. 64비트
	}

}