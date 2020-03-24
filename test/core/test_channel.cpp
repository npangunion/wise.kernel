#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/channel/channel.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/tick.hpp>

#include <vector>

using namespace wise::kernel;

struct CbHolder 
{

	~CbHolder()
	{
		if (ch_ && sk_ > 0)
		{
			ch_->unsubscribe(sk_);
		}

		WISE_INFO("~CbHolder");
	}

	void sub(channel::ptr ch, const topic& pic)
	{
		ch_ = ch;
		sk_ = ch->subscribe(pic, [this](message::ptr m) { called(); });
	}

	void called()
	{
		WISE_INFO("CbHolder called");

		v.push_back(3);
	}

	std::vector<int> v;
	skey_t sk_;
	channel::ptr ch_;
};

TEST_CASE("channel")
{

	SECTION("usage")
	{
		// basic usage
		{
			auto ch1 = channel::create("ch1", wise::kernel::channel::config());

			auto sk = ch1->subscribe(
				topic{ 1, 2, 3 },
				[](message::ptr m) { WISE_INFO("{}", m->get_topic().get_key()); }
			);

			ch1->publish(
				wise_shared<message>(topic{ 1, 2, 3 })
			);

			ch1->execute();

			CHECK(ch1->get_stat().total_post_count == 1);

			ch1->unsubscribe(sk);

			channel::destroy("ch1");
		}

		// group subscription
		{
			auto ch1 = channel::create("ch1", wise::kernel::channel::config());

			auto sk1 = ch1->subscribe(
				topic{ 1, 2, 0 },
				[](message::ptr m) { WISE_INFO("Group. {}", m->get_topic().get_key()); }
			);

			auto sk2 = ch1->subscribe(
				topic{ 1, 2, 0 },
				[](message::ptr m) { WISE_INFO("Group. {}", m->get_topic().get_key()); }
			);

			ch1->publish(
				wise_shared<message>(topic{ 1, 2, 3 })
			);

			ch1->publish(
				wise_shared<message>(topic{ 1, 2, 5 })
			);

			ch1->publish(
				wise_shared<message>(topic{ 1, 3, 3 })
			);

			ch1->execute();

			CHECK(ch1->get_stat().total_post_count == 2 * 2);

			ch1->unsubscribe(sk1);
			ch1->unsubscribe(sk2);

			channel::destroy("ch1");
		}
	}

	SECTION("channel cb binder")
	{
		auto ch1 = channel::create("ch1", wise::kernel::channel::config());

		topic pic{ 1, 2, 3 };

		{

			ch1->subscribe(
				pic, 
				[](message::ptr m) { WISE_INFO("{}", m->get_topic().get_key()); }
			);

			ch1->publish(
				wise_shared<message>(pic)
			);

			ch1->execute();

			CHECK(ch1->get_stat().total_post_count == 1);
		}

		// CHECK(ch1->get_subscription_count(pic) == 0);

		channel::destroy("ch1");
	}

	SECTION("channel pointer key")
	{
		auto ch1 = channel::create("ch1", wise::kernel::channel::config());

		auto ch2 = channel::find(ch1->get_pkey());

		CHECK(ch2);
		CHECK(ch1->get_key() == ch2->get_key());

		channel::destroy(ch2->get_key());
	}

	SECTION("performance")
	{

		SECTION("simple")
		{
			const int test_count = 1;

			auto ch1 = channel::create("ch1", wise::kernel::channel::config());

			auto sk1 = ch1->subscribe(
				topic{ 1, 2, 0 },
				[](message::ptr m) { }
			);

			auto sk2 = ch1->subscribe(
				topic{ 1, 2, 0 },
				[](message::ptr m) { }
			);

			fine_tick tick;

			for (int i = 0; i < test_count; ++i)
			{
				ch1->publish(
					wise_shared<message>(topic{ 1, 2, 5 })
				);
				ch1->execute();
			}

			WISE_INFO("channel. simple. elapsed : {}", tick.elapsed());

			ch1->unsubscribe(sk1);
			ch1->unsubscribe(sk2);

			channel::destroy("ch1");

			// 1백만. 896ms (전혀 부하가 없는 콜백) 
			// check_delayed_sub_before_enqueue를 false로 하고 741ms
			// 

			// 
			// spdlog::get() 함수가 오버헤드가 좀 크다. 
			// 로그를 자주 남기는 클래스는 미리 가져올 필요가 있다. 
			// submap에서 로그 제거하고 557ms
			// 로그는 느리지 않는데 갖고 오는 게 느리다. 
			//
		}
	}

	SECTION("error handling")
	{
		SECTION("no subscription")
		{
			auto ch1 = channel::create("ch1", wise::kernel::channel::config());

			auto sk = ch1->subscribe(
				topic{ 1, 2, 3 },
				[](message::ptr m) { WISE_INFO("{}", m->get_topic().get_key()); }
			);

			ch1->publish(
				wise_shared<message>(topic{ 1, 2, 9 })
			);

			ch1->execute();

			CHECK(ch1->get_stat().total_post_count == 0);

			ch1->unsubscribe(sk);

			channel::destroy("ch1");
		}

		SECTION("lambda w/ null reference")
		{
			auto cb = wise_shared<CbHolder>();

			auto ch1 = channel::create("ch1", wise::kernel::channel::config());

			cb->sub(ch1, topic{ 1, 2, 9 });

			ch1->publish(
				wise_shared<message>(topic{ 1, 2, 9 })
			);

			void* p = (void*)cb.get();

			cb.reset(); // delete the cb object

			::memset(p, 0, 16);

			ch1->execute();

			CHECK(ch1->get_stat().total_post_count == 0);

			channel::destroy("ch1");

			// 람다가 this 주소를 잘 갖고 있다. 

			// 메모리 깨지면 문제가 생기고 버그의 소지도 있어 
			// 꼭 unsubscribe를 해줘야 한다. 매우 치명적일 수 있다. 
			// - weak_ptr()을 람다의 참조로 갖고 레퍼런스를 관리해도 된다. 
			// - 소멸자에서 unsubscribe 하는 구조를 하나 만들어도 된다.
			// 
		}
	}
}