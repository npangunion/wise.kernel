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

		auto ta = [this](timer::id_t ti) { 
			WISE_UNUSED(ti);  
			on_timer(); 
		};

		auto id = get_timer().set(100);
		get_timer().add(id, ta);

		return true;
	}

	result run()
	{
		return result::success;
	}

	void on_timer()
	{
		WISE_INFO("actor:{:x}. on_timer.", get_id());
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
	SECTION("id")
	{
		actor_id_generator<> gen;
		gen.setup(16001);

		auto id = gen.next();
		CHECK(gen.get_domain_of(id) == 16001);
	}

	SECTION("creation")
	{
		server s;

		simple_actor sa(s, 1, "name");

		CHECK(sa.get_id() == 1);
	}


}




