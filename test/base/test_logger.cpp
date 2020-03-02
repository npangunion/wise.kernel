#include "pch.hpp"
#include <catch.hpp>
#include <wise.kernel/base/logger.hpp>
#include <wise.kernel/base/tick.hpp>

class LogFunction
{
public:
	static void Log()
	{
		WISE_INFO("Log from LogFunction");
	}

};

TEST_CASE("logger")
{
	SECTION("creating directory from fileprefix")
	{
		// 	
	}

	SECTION("simple interface")
	{
		wise::kernel::log()->info("Hello logger!");
	}

	SECTION("macro")
	{
		WISE_INFO("Hello log with macro!");
		WISE_INFO("Hello log with macro! value: {}", 3);

		LogFunction::Log();
	}

	SECTION("wchar support")
	{
		// UTF8로 변환하므로, 파일은 정확하게 보이고, 콘솔은 깨져 보인다. 

		// SPDLOG_WCHAR_TO_UTF8_SUPPORT이 spdlog/teakme.h에 켜져 있어야 한다. 

		WISE_INFO(L"Hello {}", L"한글");

		// 파일에서 확인 : OK
	}

	SECTION("performance")
	{
		// 약식 성능 테스트
		const int test_count = 1;

		wise::kernel::fine_tick tick;

		for (int i = 0; i < test_count; ++i)
		{
			WISE_INFO("A simple log performance test. index: {}", i);
		}

		WISE_INFO("Elapsed: {}", tick.elapsed());

		// 파일 로그: 10만건, 123ms
		// 콘솔 로그: 10만건, 8초 

		// 서비스 할 때 콘솔 로그 제외해야 함
	}

	SECTION("manual verification")
	{
		// WISE_ENABLE_LOG_TRACE 

	}
}