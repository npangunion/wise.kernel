#include <pch.hpp>
#include <catch.hpp>

namespace {

	struct addr {
		uint8_t					depth;
		std::array<uint16_t, 3>	pars;

		uint16_t operator[] (uint8_t index)
		{
			WISE_THROW_IF(index >= depth, "index is out of range");

			return pars[index];
		}
	};
}

TEST_CASE("design")
{
	SECTION("usage")
	{
		addr a1{ 2, {1, 912} };

		CHECK(a1[0] == 1);
		CHECK(a1[1] == 912);
	}
}