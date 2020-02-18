#include <catch.hpp>
#include <wise.kernel/base/macros.hpp>

TEST_CASE("macros", "wise.kernel.base.macros")
{
	REQUIRE(1 == 1);

	SECTION("coverage")
	{
		SECTION("1")
		{
			REQUIRE(1 != 1);
		}
	}
}
