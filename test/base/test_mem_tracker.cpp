#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/base/mem_tracker.hpp>

namespace
{
	class simple
	{
	public: 
		simple(int v)
			: v_(v)
		{}

	private: 
		int v_;
	};
}

using namespace wise::kernel;

TEST_CASE("memory_tracker", "base")
{
	SECTION("basic functionality")
	{
		auto p = wise_new<simple>(3);

		mem_tracker::inst().report();

		delete p;

		mem_tracker::inst().report();

		// 로그 파일 내용. 
/* 
[2020-03-11 17:09:09.752][23672][I]
--------------------------------------------------------------------
	memory report
--------------------------------------------------------------------
size stats:
	size:            4	count:            1
type stats:
	size:            4	count:            1	type: class `anonymous namespace'::simple

current      :                0 KB
total alloc  :                0 KB
total dealloc:                0 KB

[2020-03-11 17:09:09.752][23672][I]
--------------------------------------------------------------------
	memory report
--------------------------------------------------------------------
size stats:
type stats:

current      :                0 KB
total alloc  :                0 KB
total dealloc:                0 KB
*/

	}

	SECTION("performance")
	{
		const int TEST_COUNT = 1000000;

		SECTION("tracked")
		{
			fine_tick tick;

			for (int i = 0; i < TEST_COUNT; ++i)
			{
				auto p = wise_new<simple>(3);
				delete p;
			}

			mem_tracker::inst().report();

			std::vector<simple*> v;

			for (int i = 0; i < TEST_COUNT; ++i)
			{
				auto p = wise_new<simple>(3);
				v.push_back(p);
			}

			mem_tracker::inst().report();

			for (auto& s : v)
			{
				delete s;
			}

			mem_tracker::inst().report();

			WISE_INFO("mem_tracker. tracked. elapsed: {}", tick.elapsed());
		}

		SECTION("untracked")
		{
			fine_tick tick;

			for (int i = 0; i < TEST_COUNT; ++i)
			{
				auto p = new simple(3);
				delete p;
			}

			mem_tracker::inst().report();

			std::vector<simple*> v;

			for (int i = 0; i < TEST_COUNT; ++i)
			{
				auto p = new simple(3);
				v.push_back(p);
			}

			mem_tracker::inst().report();

			for (auto& s : v)
			{
				delete s;
			}

			mem_tracker::inst().report();

			WISE_INFO("mem_tracker. untracked. elapsed: {}", tick.elapsed());
		}

		SECTION("disabled dynamic") // config.hpp에서 WISE_TRACK_MEMORY 0으로 빌드
		{
			mem_tracker::inst().disable();

			fine_tick tick;

			for (int i = 0; i < TEST_COUNT; ++i)
			{
				auto p = new simple(3);
				delete p;
			}

			mem_tracker::inst().report();

			std::vector<simple*> v;

			for (int i = 0; i < TEST_COUNT; ++i)
			{
				auto p = new simple(3);
				v.push_back(p);
			}

			mem_tracker::inst().report();

			for (auto& s : v)
			{
				delete s;
			}

			mem_tracker::inst().report();

			mem_tracker::inst().enable();

			WISE_INFO("mem_tracker. disabled dynamic. elapsed: {}", tick.elapsed());
		}


		SECTION("disabled compile time") // config.hpp에서 WISE_TRACK_MEMORY 0으로 빌드
		{
			fine_tick tick;

			for (int i = 0; i < TEST_COUNT; ++i)
			{
				auto p = new simple(3);
				delete p;
			}

			mem_tracker::inst().report();

			std::vector<simple*> v;

			for (int i = 0; i < TEST_COUNT; ++i)
			{
				auto p = new simple(3);
				v.push_back(p);
			}

			mem_tracker::inst().report();

			for (auto& s : v)
			{
				delete s;
			}

			mem_tracker::inst().report();

			WISE_INFO("mem_tracker. disabled compile time. elapsed: {}", tick.elapsed());
		}

		// 백만. 릴리스 모드  
		//  - tracked   : 2098 ms
		//  - untracked :  327 ms 
		//  - disabled (dynamic)  :  149 ms
		//  - disabled (compile)  :  135 ms

		// 권장 사용법 
		// - compile 할 때 WISE_TRACK_MEMORY 1 로 켬
		// - 실행할 때 동적으로 켜거나 끔
	}
}