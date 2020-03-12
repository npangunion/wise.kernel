#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/result.hpp>

using namespace wise::kernel;

namespace
{
	enum result_code
	{
		success,
		fail_1, 
		fail_2, 
	};
}

TEST_CASE("result", "base")
{
	SECTION("usage")
	{
		SECTION("bool, int")
		{
			result<bool, int> code(false, 3);

			REQUIRE(!code);
			REQUIRE(code.value == 3);
		}

		SECTION("int, string")
		{
			result<int, std::string> code(false, "hello");

			REQUIRE(!code);
			REQUIRE(code.value == "hello");
		}

		SECTION("bool, enum")
		{
			result<bool, result_code> code(false, result_code::fail_2);

			REQUIRE(!code);
			REQUIRE(code.value == result_code::fail_2);
		}
	}
}
