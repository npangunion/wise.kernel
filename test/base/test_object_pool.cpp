#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/base/object_pool.hpp>

using namespace wise::kernel;

namespace
{
	class test_object
	{
	public: 
		test_object(const std::string& name, int age)
			: name_(name)
			, age_(age)
		{
		}

		const std::string& name() const 
		{
			return name_;
		}

		int age() const
		{
			return age_;
		}

	private:
		std::string name_;
		int age_;
	};
}

TEST_CASE("object_pool", "base")
{
	SECTION("usage")
	{
		SECTION("ref")
		{
			object_pool<test_object> pool("usage1");

			// alloc and free with ref
			{
				auto ref = pool.construct("me", 3);
				REQUIRE(pool.alloc_count() == 1);
			}

			REQUIRE(pool.alloc_count() == 0);
		}

		SECTION("shared ref")
		{
			object_pool<test_object> pool("usage1");

			// alloc and free with ref
			{
				auto ref = pool.construct_shared("me", 3);
				REQUIRE(pool.alloc_count() == 1);
			}

			REQUIRE(pool.alloc_count() == 0);
		}

		SECTION("ref in containers")
		{
			SECTION("vector")
			{
				object_pool<test_object> pool("vector");

				std::vector<object_pool<test_object>::ref> refs;

				refs.push_back(pool.construct("me", 1));
				refs.push_back(pool.construct("you", 3));
				refs.push_back(pool.construct("k", 100));

				REQUIRE(refs[0]->name() == "me");
				REQUIRE(refs[1]->name() == "you");

				refs.erase(refs.begin());

				REQUIRE(refs.size() == 2);
				REQUIRE(pool.alloc_count() == 2);
			
				REQUIRE(refs[0]->name() == "you");
				REQUIRE(refs[1]->name() == "k");
			}

			SECTION("map")
			{
				object_pool<test_object> pool("map");

			}

		}
	}

	SECTION("leak check")
	{
		// pool scope
		{
			object_pool<test_object> pool("leak1");

			auto p = pool.construct_raw("me", 3);
			REQUIRE(pool.alloc_count() == 1);
		}

		// logged in system log 
	}

	// performance?
}
