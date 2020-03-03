#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/base/stacktrace.hpp>


TEST_CASE("stacktrace", "[base]")
{
	SECTION("execution")
	{
		wise::kernel::stacktrace sp;

		REQUIRE(sp.dump("stacktrace execution").size() > 0);

		// it's really slow. don't use it!!!
	}
}
