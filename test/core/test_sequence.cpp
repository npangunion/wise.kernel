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
		// release�� ������ �������� �������� ���ɼ��� �ִ�. 
		// 
	}
}

