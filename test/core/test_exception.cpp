#include <pch.hpp>
#include <catch.hpp>

#pragma warning(disable: 4702)

TEST_CASE("exception")
{
	SECTION("throw w/ a macro")
	{
		REQUIRE_THROWS(WISE_THROW("just an exception"));
		REQUIRE_THROWS(WISE_THROW_FMT("Exception: {}", "Hello")); 
	}
}
