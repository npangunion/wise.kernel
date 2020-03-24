#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/net/util/slot_vector.hpp>

using namespace wise::kernel;

TEST_CASE("slot_vector", "net")
{
	SECTION("usage")
	{
		using svs = slot_vector<std::string>;
		svs sv;

		auto id = sv.add(std::make_shared<svs::value_type>("Hello"));

		svs::slot_id sid(id);

		CHECK(sid.get_index() == 0);
		CHECK(sid.get_seq() == 1);

		sv.del(id);

		auto id2 = sv.add(std::make_shared<svs::value_type>("Hello"));

		svs::slot_id sid2(id2);

		CHECK(sid2.get_index() == 0);
		CHECK(sid2.get_seq() == 2);

		auto sp2 = sv.get(id2);
		CHECK(*sp2 == "Hello");

		auto sp1 = sv.get(id);
		CHECK(!sp1);

		CHECK(sv.get_used_count() == 1);
		sv.del(id2);
		CHECK(sv.get_used_count() == 0);

		CHECK(sv.get_capacity() == 1);
	}
}