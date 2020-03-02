#include "pch.hpp"
#include <catch.hpp>
#include <wise.kernel/base/concurrent_queue.hpp>
#include <wise.kernel/base/tick.hpp>
#include <wise.kernel/base/logger.hpp>
#include <thread>

struct message
{
	int mem[128];
	int id;

	using ptr = std::shared_ptr<message>;
};

TEST_CASE("concurrent queue")
{
	SECTION("usage")
	{
		wise::kernel::concurrent_queue<message::ptr> cq;

		auto p1 = std::make_shared<message>();

		p1->id = 1;

		cq.push(p1);

		message::ptr p;

		CHECK(cq.pop(p));

		CHECK(p->id == 1);
	}

	SECTION("performance")
	{
		// 4 threads on 8 core machine 
		// 2 producer, 2 consumer
		// 
		std::atomic<int> count = 0;

		wise::kernel::concurrent_queue<message::ptr> cq;
		wise::kernel::concurrent_queue<message::ptr> pool;

		int test_count = 1000;

		for (int i = 0; i < test_count * 2; ++i)
		{
			pool.push(std::make_shared<message>());
		}

		auto producer = [&cq, test_count, &pool]()
		{
			for (int i = 0; i < test_count; ++i)
			{
				message::ptr p1;

				pool.pop(p1);

				p1->id = 1;

				cq.push(p1);
			}
		};

		auto consumer = [&cq, &count, test_count]()
		{
			for (int i = 0; i < test_count; ++i)
			{
				++count;
				message::ptr p;
				cq.pop(p);
			}
		};

		std::thread p1(producer);
		std::thread p2(producer);

		std::thread c1(consumer);
		std::thread c2(consumer);


		using namespace std::chrono_literals;

		wise::kernel::fine_tick ft;

		while (count < test_count * 2)
		{
			std::this_thread::sleep_for(1ms);
		}

		WISE_INFO("concurrent_queue elapsed: {}", ft.elapsed());

		p1.join();
		p2.join();
		c1.join();
		c2.join();
	}
}