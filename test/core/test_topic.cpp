#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/topic.hpp>

TEST_CASE("topic")
{

	SECTION("usage")
	{
		{
			wise::kernel::topic pic(1, 1, 1);
			// wise::topic pic2(-1, -2, -3); // 컴파일러 경고 나와야 함.
		}

		{
			wise::kernel::topic pic(2, 3, 4);

			CHECK(pic.get_category() == 2);
			CHECK(pic.get_group() == 3);
			CHECK(pic.get_type() == 4);

			CHECK(pic.get_key() == 0x02030004);
		}

		{
			wise::kernel::topic pic(1, 2, 3);
			wise::kernel::topic pic2(1, 2, 3);

			CHECK(pic == pic2);
			CHECK((pic != pic2) == false);
		}

		{
			wise::kernel::topic pic(1, 2, 3);

			auto pic2 = pic;

			CHECK(pic == pic2);

			wise::kernel::topic pic3(2, 1, 2);

			CHECK(pic3 > pic);
			CHECK(pic < pic3);
		}
	}
}