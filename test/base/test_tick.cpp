#include <catch.hpp>
#include <wise.kernel/base/tick.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

TEST_CASE("tick")
{

	SECTION("fine tick")
	{
		wise::kernel::fine_tick tick;

		CHECK(tick.elapsed() < 1);

		std::this_thread::sleep_for(110ms); // Ÿ�ֿ̹� ���� ������ �� �ִ�.

		CHECK(tick.elapsed() >= 100);

		//
		// QueryPerformanceFrequency / QueryPerformanceCounter ���
	}

	SECTION("simple tick")
	{
		wise::kernel::simple_tick tick;

		CHECK(tick.elapsed() < 1);

		std::this_thread::sleep_for(110ms); // Ÿ�ֿ̹� ���� ������ �� �ִ�.

		CHECK(tick.elapsed() >= 100);

		CHECK(tick.check_timeout(100, false) == true);
		CHECK(tick.check_timeout(100, true) == true);
		CHECK(tick.check_timeout(100) == false);

		// �Ʒ� �������� ����Ѵ�. 
		// if ( tick.check_timeout(100) )
		// {
		//		// do something
		// }

		// ƽ ī��Ʈ ���. 64��Ʈ
	}

}