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
		// UTF8�� ��ȯ�ϹǷ�, ������ ��Ȯ�ϰ� ���̰�, �ܼ��� ���� ���δ�. 

		// SPDLOG_WCHAR_TO_UTF8_SUPPORT�� spdlog/teakme.h�� ���� �־�� �Ѵ�. 

		WISE_INFO(L"Hello {}", L"�ѱ�");

		// ���Ͽ��� Ȯ�� : OK
	}

	SECTION("performance")
	{
		// ��� ���� �׽�Ʈ
		const int test_count = 1;

		wise::kernel::fine_tick tick;

		for (int i = 0; i < test_count; ++i)
		{
			WISE_INFO("A simple log performance test. index: {}", i);
		}

		WISE_INFO("Elapsed: {}", tick.elapsed());

		// ���� �α�: 10����, 123ms
		// �ܼ� �α�: 10����, 8�� 

		// ���� �� �� �ܼ� �α� �����ؾ� ��
	}

	SECTION("manual verification")
	{
		// WISE_ENABLE_LOG_TRACE 

	}
}