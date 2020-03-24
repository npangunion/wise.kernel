#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/channel/channel.hpp>

#pragma warning(disable: 6319)

using namespace wise::kernel;

TEST_CASE("subscriptions", "channel")
{

	SECTION("sub")
	{
		SECTION("posted")
		{
			sub::cond_t true_ = [](message::ptr m) { return true; };

			int v = 0;
			sub::cb_t cb = [&v](message::ptr m) { ++v; };

			sub s(0, topic(1, 1, 1), true_, cb, sub::mode::immediate);

			bool rc = s.post(std::make_shared<message>(topic(1, 1, 1)));
			CHECK(rc);
		}

		SECTION("not matched")
		{
			sub::cond_t true_ = [](message::ptr m) { return true; };

			int v = 0;
			sub::cb_t cb = [&v](message::ptr m) { ++v; };

			sub s(0, topic(1, 1, 1), true_, cb, sub::mode::immediate);

			bool rc = s.post(std::make_shared<message>(topic(1, 0, 1)));
			CHECK(!rc);
		}

		SECTION("conditin is false")
		{
			sub::cond_t false_ = [](message::ptr m) { return false; };

			int v = 0;
			sub::cb_t cb = [&v](message::ptr m) { ++v; };

			sub s(0, topic(1, 1, 1), false_, cb, sub::mode::immediate);

			bool rc = s.post(std::make_shared<message>(topic(1, 1, 1)));
			CHECK(!rc);
		}
	}

	SECTION("sub_map")
	{
		channel ch("hello");

		sub_map sm(ch);

		auto fn = [](message::ptr m) {};

		auto pic = topic(2, 2, 2);
		auto pic2 = topic(2, 2, 3);

		auto key = sm.subscribe(pic, fn, sub::mode::delayed);
		CHECK(key > 0);

		auto cnt = sm.post(wise_shared<message>(pic), sub::mode::delayed);
		CHECK(cnt == 1);

		cnt = sm.post(pic2, wise_shared<message>(pic), sub::mode::delayed);
		CHECK(cnt == 0);

		sm.unsubscribe(key);
		auto cnt2 = sm.post(wise_shared<message>(pic), sub::mode::delayed);
		
		CHECK(cnt2 == 0);
	}
}