#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/util/crc32.hpp>

TEST_CASE("crc32", "util")
{
	SECTION("usage")
	{
		std::string data("Hello Crc32");
		
		uint32_t crc1 = wise::kernel::crc32(data.begin(), data.end());

		WISE_INFO("crc:{}", crc1);
	}

}