#include "pch.hpp"
#include <catch.hpp>
#include <wise.kernel/core/spinlock.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/tick.hpp>

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
		// constexpr int test_count = 1000000; // �鸸 
		constexpr int test_count = 100;

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

		// 1�鸸��. thread 4 vec.push_back()
		// spin: 95ms, recursive_mutex: 157ms
		// 
		// ��Ȳ�� ���� �پ��ϰ� �ٸ��Ƿ� �׻� �����ٰ� �� ���� ����. 
		// �ſ� ª�� ����Ǵ� ���� ���� ���ɶ��� ���� �� �ִ�. 
		// 
		// �Ｚ��ž. ������: 1�鸸�� 
		// [2020-03-03 23:54:41.605][3940][I] spinlock. elapsed: 122 ms
		// [2020-03-03 23:54:41.742][3940][I] mutex. elapsed: 135 ms
	}
}
