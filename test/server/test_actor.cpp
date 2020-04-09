#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/server/server.hpp>

using namespace wise::kernel;

namespace
{

class simple_actor : public actor
{
public:
	simple_actor(server& _server, id_t id, const std::string& name)
		: actor(_server, id)
		, name_(name)
	{
	}

private: 
	bool init()
	{
		WISE_INFO("simple_actor:{:x} start", get_id());

		auto ta = [this](timer::id_t i) { WISE_INFO("simple_actor:{:x} timer", get_id()); };
		get_timer().once(10, ta);

		return true;
	}

	void fini()
	{
		WISE_INFO("simple_actor:{:x} finish", get_id());
	}

private: 
	std::string name_;
};

} // noname

TEST_CASE("actor", "server")
{

	SECTION("creation")
	{
		server s;

		simple_actor sa(s, 1, "name");

		CHECK(sa.get_id() == 1);
		CHECK(sa.get_parent() == 0);
	}

	SECTION("actor from server")
	{
		server s; 

		if (s.start())
		{
			auto ap = s.create<simple_actor>("simple_actor");

			CHECK(s.get_local_actor(ap->get_id()) == ap);

			s.run();

			sleep(1000);

			s.finish();
		}
	}


}




