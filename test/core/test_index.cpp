#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/index.hpp>

using namespace wise::kernel;

namespace
{
	struct dummy1
	{
		int v = 0;

		dummy1()
		{
		}

		dummy1(int v)
		{
			this->v = v;
		}
	};
}

TEST_CASE("index", "base")
{
	SECTION("usage")
	{
		index<int, int> i1; 

		i1.set(0, 5);
		REQUIRE(i1.has(0));
		REQUIRE(i1.get(0) == 5);
		REQUIRE(i1.size() == 1);

		i1.set(1, 3);
		REQUIRE(i1.has(1));
		REQUIRE(i1.get(1) == 3);
		REQUIRE(i1.size() == 2);

		if (i1.has(0))
		{
			i1.unset(0);
			REQUIRE(i1.has(0) == false);
			REQUIRE(i1.size() == 1);
		}

		i1.clear();

		REQUIRE(i1.has(1) == false);
		REQUIRE(i1.size() == 0);
		
		REQUIRE(i1.get(1) == int());
		REQUIRE(int() == 0);
	}

	SECTION("converage")
	{
		SECTION("value types")
		{
			SECTION("string")
			{
				index<int, std::string> i2;

				i2.set(0, "hello");
				REQUIRE(i2.get(0) == "hello");
				REQUIRE(i2.get(1) == std::string());
				REQUIRE(i2.get(1) == "");
			}

			SECTION("float")
			{
				index<int, float> i2;

				i2.set(0, 0.3f);
				REQUIRE(i2.get(0) == 0.3f);
				REQUIRE(i2.get(1) == float());
				REQUIRE(i2.get(1) == 0);
			}

			SECTION("struct")
			{
				index<int, dummy1> i2;

				i2.set(0, dummy1(3));
				REQUIRE(i2.get(0).v == 3);
			}
		}
	}
}
