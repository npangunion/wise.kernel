#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/server/server.hpp>
#include <wise.kernel/server/actor_factory.hpp>
#include <wise.kernel/util/json.hpp>
#include <fstream>

#pragma warning(disable: 6319)
#pragma warning(disable: 6327)

using namespace wise::kernel;
using namespace nlohmann;

namespace {

class simple_actor : public actor
{
public:
	simple_actor(server& _server, id_t id)
		: actor(_server, id)
	{}

	bool init() override 
	{
		return true;
	}

	void fini() override
	{
	}
};

} // noname

TEST_CASE("server", "server")
{
	SECTION("basic flow")
	{
		// register actor 
		WISE_ADD_ACTOR(simple_actor);

		SECTION("actor_factory")
		{
			CHECK(actor_factory::inst().has("simple_actor"));

			server s1;
			auto ap = actor_factory::inst().create("simple_actor", s1, 1);
			CHECK(ap->get_id() == 1);
		}

		SECTION("actors are created from a json configuration file in a server")
		{
			server s1;

			auto rc = s1.start("../test/server/config_actors.json");
			CHECK(rc);

			auto aref = s1.get_actor("simple_1");
			CHECK(aref.get_id() > 0);

			s1.finish();
		}
	}

	SECTION("actor from server")
	{
		server s;

		if (s.start("../test/server/config_actors.json"))
		{
			auto aref = s.create<simple_actor>();

			CHECK(s.get_actor(aref.get_id()) == aref);

			s.run();

			s.finish();
		}
	}

	SECTION("actor with name")
	{
		server s;

		if (s.start("../test/server/config_actors.json"))
		{
			auto aref = s.create_with_name<simple_actor>("simple_2");

			CHECK(s.get_actor("simple_2") == aref);

			s.run();

			s.finish();
		}
	}

	SECTION("actor execution")
	{
		// timer, channel

		server s;

		if (s.start("../test/server/config_actors.json"))
		{
			auto aref = s.create<simple_actor>();

			CHECK(s.get_actor(aref.get_id()) == aref);

			for (int i = 0; i < 1000; ++i)
			{
				s.run();
				sleep(1);
			}

			s.finish();
		}
	}

	SECTION("bits interface")
	{
		
	}
}


TEST_CASE("nlohmann", "json")
{
	// to refresh memory

	SECTION("parse and use")
	{
		std::string file("..\\test\\server\\config_refresh.json");

		std::ifstream fs(file);

		if (!fs.is_open())
		{
			WISE_THROW_FMT(fmt::format("File [{}] not found", file));
		}

		auto cfg = nlohmann::json::parse(fs);
		fs.close();

		CHECK(!cfg["name"].is_null());

		// get value
		CHECK(cfg["name"].get<std::string>() == "test");

		CHECK(!cfg["array"].is_null());

		// iterate an array
		for (auto& a : cfg["array"])
		{
			CHECK(!a["service"].is_null());
			CHECK(!a["service"]["type"].is_null());
		}
		
	}
}