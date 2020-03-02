#include "pch.hpp"
#include <catch.hpp>
#include <wise.kernel/base/macros.hpp>

TEST_CASE("macros", "wise.kernel.base.macros")
{
	bool test_assert_fail = false;

	SECTION("assert")
	{
		WISE_ASSERT(1 == 1);

		if (test_assert_fail)
		{
			// tested once
			WISE_ASSERT(1 != 1);
		}
	}

	SECTION("expect/ensure")
	{
		WISE_EXPECT(1 == 1); // bool case
		WISE_ENSURE(1 == 1); // bool case
		WISE_EXPECT("string case");
		WISE_ENSURE("string case");
		WISE_EXPECT(1); // number
		WISE_ENSURE(1); // number
		WISE_EXPECT(1.0f); // number
		WISE_ENSURE(1.0f); // number
	}

	SECTION("return_if")
	{
		int check_value = 1;

		auto func1 = [&check_value]() {
			WISE_RETURN_IF(true, 3);
			check_value = 5;
		};

		func1();

		CHECK(check_value == 1);

		auto func2 = [&check_value]() {
			check_value = 2;
			WISE_RETURN_IF(true);
			check_value = 10;
		};

		func2();

		CHECK(check_value == 2);

		auto func3 = [&check_value]() {
			WISE_RETURN_IF(false);
			check_value = 9;
		};

		func3();

		CHECK(check_value == 9);
	}

	SECTION("return_call_if")
	{
		int check_value = 0;

		auto func1 = [&check_value]() {
			check_value = 5;
		};

		auto func2 = [&check_value, func1]() {
			check_value = 2;
			WISE_RETURN_CALL_IF(true, 0, func1);
			check_value = 10;
		};

		func2(); 

		CHECK(check_value == 5);
	}
}
