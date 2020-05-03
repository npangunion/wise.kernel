#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/server/server.hpp>

using namespace wise::kernel;

TEST_CASE("actor_cluster_service", "server")
{
	SECTION("actor_cluster_service")
	{
		server s1;
		server s2;

		s1.start("../test/server/config_peer_1.json");
		s2.start("../test/server/config_peer_2.json");

		for (int i = 0; i < 1000; ++i)
		{
			s1.run();
			s2.run();

			sleep(1);
		}

		auto ps1 = s1.get_actor("actor_cluster_service");
		auto ps2 = s2.get_actor("actor_cluster_service");

		// check

		s1.finish();

		for (int i = 0; i < 1000; ++i)
		{
			s2.run();

			sleep(1);
		}

		s2.finish();
	}

	SECTION("actor_cluster_service and public actors")
	{
		// public actor announces itself to peers w/ syn_actor_up
		// 
	}
}