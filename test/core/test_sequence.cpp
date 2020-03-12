#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/sequence.hpp>

using namespace wise::kernel;

TEST_CASE("sequence", "base")
{

	SECTION("usage")
	{
		sequence<int> seq(1, INT32_MAX);

		for (int i = 0; i < 100000; ++i)
		{
			REQUIRE(seq.next() == i+1);
		}

		int v = seq.next();
		seq.release(v);

		//
		// release를 잊으면 시퀀스가 부족해질 가능성이 있다. 
		// 
	}
}

