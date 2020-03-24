#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/channel/sub_map.hpp>

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

	}
}