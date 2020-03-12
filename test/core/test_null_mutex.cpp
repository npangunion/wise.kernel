#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/core/null_mutex.hpp>

using namespace wise::kernel;

TEST_CASE("null_mutex", "base")
{
	SECTION("usage")
	{
		SECTION("lock")
		{
			null_mutex nm; 

			nm.lock(); 
			REQUIRE(nm.locked == 1);

			nm.unlock();
			REQUIRE(nm.locked == 0);
		}

		SECTION("try_lock")
		{
			null_mutex nm; 

			REQUIRE(nm.try_lock()); 
			REQUIRE(nm.locked == 1);

			nm.unlock();
			REQUIRE(nm.locked == 0);
		}

		SECTION("lock_guard")
		{
			null_mutex nm;

			// lock scope
			{
				std::lock_guard<null_mutex> lock(nm);
				REQUIRE(nm.locked == 1);
			}

			REQUIRE(nm.locked == 0);
		}
	}
}
