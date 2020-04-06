#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/fmt.hpp>
#include <wise.kernel/core/suid_generator.hpp>

namespace {

	struct addr {
		uint8_t					depth;
		std::array<uint16_t, 3>	pars;

		uint16_t operator[] (uint8_t index)
		{
			WISE_THROW_IF(index >= depth, 
				fmt::format("index is out of range. {}/{}", index, depth).c_str()
			);
			return pars[index];
		}
	};
}

TEST_CASE("design")
{
	SECTION("design 1")
	{
		addr a1{ 2, {1, 912} };

		CHECK(a1[0] == 1);
		CHECK(a1[1] == 912);
	}

	SECTION("suid")
	{
		wise::kernel::suid_generator generator;
		generator.setup(1);

		CHECK(generator.next() <= generator.next());

		// 64bit snowflake id가 편하고 효율적이다. 
	}
}