#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/server/server.hpp>

using namespace wise::kernel;

TEST_CASE("peer_service", "server")
{
	SECTION("startup")
	{
		server s1;

		server s2;
	}
}