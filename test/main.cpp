#include "pch.hpp"
#define CATCH_CONFIG_COLOUR_WINDOWS
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <wise.kernel/base/logger.hpp>

int main(int argc, char* argv[])
{
    wise::kernel::system_logger::enable_console = false;

    Catch::Session session; 

    int returnCode = session.applyCommandLine(argc, argv);

    if (returnCode != 0) 
    {
        return returnCode;
    }

    int numFailed = session.run();
    return numFailed;
}