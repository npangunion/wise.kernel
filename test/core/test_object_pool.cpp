#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/object_pool.hpp>

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
		SECTION("shared ref")
		{
			object_pool<test_object> pool("usage1");

			// alloc and free with ref
			{
				auto ref = pool.create("me", 3);
				REQUIRE(pool.alloc_count() == 1);
			}

			REQUIRE(pool.alloc_count() == 0);
		}

		SECTION("ref in containers")
		{
			SECTION("vector")
			{
				object_pool<test_object> pool("vector");

				using vec = std::vector<object_pool<test_object>::shared_ref>;
				
				vec refs;

				refs.push_back(pool.create("me", 1));
				refs.push_back(pool.create("you", 3));
				refs.push_back(pool.create("k", 100));

				REQUIRE(refs[0]->name() == "me");
				REQUIRE(refs[1]->name() == "you");

				// ref
				{
					auto r1 = refs[0];

					REQUIRE(r1->name() == "me");
				}

				refs.erase(refs.begin());

				REQUIRE(refs.size() == 2);
				REQUIRE(pool.alloc_count() == 2);
			
				REQUIRE(refs[0]->name() == "you");
				REQUIRE(refs[1]->name() == "k");
			}

			SECTION("vector copy")
			{
				object_pool<test_object> pool("vector");

				using vec = std::vector<object_pool<test_object>::shared_ref>;
				
				vec refs;

				refs.push_back(pool.create("me", 1));
				refs.push_back(pool.create("you", 3));
				refs.push_back(pool.create("k", 100));

				REQUIRE(refs[0]->name() == "me");
				REQUIRE(refs[1]->name() == "you");

				vec refs2 = refs; // copy

				REQUIRE(refs2[1]->name() == "you");

				REQUIRE(pool.alloc_count() == 3);
			}

			SECTION("map")
			{
				object_pool<test_object> pool("map");

				using map = std::map<int, object_pool<test_object>::shared_ref>;
				
				map refs;

				refs.insert(map::value_type(1, pool.create("me", 1)));
				refs.insert(map::value_type(2, pool.create("you", 3)));
				refs.insert(map::value_type(8, pool.create("k", 100)));

				REQUIRE(refs[8]->name() == "k");
				REQUIRE(refs[8]->age() == 100);

				map refs2 = refs;

				REQUIRE(refs2[1]->name() == "me");
				REQUIRE(pool.alloc_count() == 3);

				refs2.erase(8);

				REQUIRE(pool.alloc_count() == 3);

				refs.erase(8);
				REQUIRE(pool.alloc_count() == 2);
			}
		}
	}

	SECTION("leak check")
	{
		// pool scope
		{
			object_pool<test_object> pool("leak1");

			auto p = pool.create("me", 3);
			REQUIRE(pool.alloc_count() == 1);
		}

		// logged in system log 
	}

	// performance?
}
