#include "pch.hpp"
#include <catch.hpp>
#include <wise.kernel/base/spinlock.hpp>
#include <wise.kernel/base/logger.hpp>
#include <wise.kernel/base/tick.hpp>

TEST_CASE("spinlock")
{

	SECTION("safety")
	{
		std::vector<int> vec;

		constexpr int thread_count = 10;

		std::vector<std::thread> threads;
		threads.resize(thread_count);

		wise::kernel::spinlock  sp;

		for (int i = 0; i < thread_count; ++i)
		{
			std::thread thread([i, &vec, &sp]() {

				for (int j = 0; j < 100; ++j)
				{
					std::lock_guard<wise::kernel::spinlock> lock(sp);
					vec.push_back(i + j);
				}
				});

			threads[i].swap(thread);
		}

		for (int i = 0; i < thread_count; ++i)
		{
			threads[i].join();
		}
	}


	SECTION("performance")
	{
		constexpr int test_count = 1000; // 백만 
		// constexpr int test_count = 1;

		SECTION("spinlock")
		{
			wise::kernel::fine_tick tick;

			std::vector<int> vec;
			vec.resize(test_count);

			constexpr int thread_count = 4;

			std::vector<std::thread> threads;
			threads.resize(thread_count);

			wise::kernel::spinlock  sp;

			for (int i = 0; i < thread_count; ++i)
			{
				std::thread thread([i, &vec, &sp, test_count]() {

					for (int j = 0; j < test_count; ++j)
					{
						std::lock_guard<wise::kernel::spinlock> lock(sp);
						vec.push_back(i + j);
					}
					});

				threads[i].swap(thread);
			}

			for (int i = 0; i < thread_count; ++i)
			{
				threads[i].join();
			}

			WISE_INFO("spinlock. elapsed: {}", tick.elapsed());
		}

		SECTION("recursive mutex")
		{
			wise::kernel::fine_tick tick;

			std::vector<int> vec;
			vec.resize(test_count);

			constexpr int thread_count = 4;

			std::vector<std::thread> threads;
			threads.resize(thread_count);

			std::recursive_mutex sp;

			for (int i = 0; i < thread_count; ++i)
			{
				std::thread thread([i, &vec, &sp, test_count]() {
					for (int j = 0; j < test_count; ++j)
					{
						std::lock_guard<std::recursive_mutex> lock(sp);
						vec.push_back(i + j);
					}
					});

				threads[i].swap(thread);
			}

			for (int i = 0; i < thread_count; ++i)
			{
				threads[i].join();
			}

			WISE_INFO("mutex. elapsed: {}", tick.elapsed());
		}

		// 1백만번. thread 4 vec.push_back()
		// spin: 95ms, recursive_mutex: 157ms
		// 
		// 상황에 따라 다양하게 다르므로 항상 빠르다고 할 수는 없다. 
		// 매우 짧게 실행되는 곳의 락은 스핀락이 나을 수 있다. 
		// 
	}
}
