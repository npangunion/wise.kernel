#include "stdafx.h"
#include "platform.hpp"
#include "macros.hpp" 

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

namespace r2c {

std::string platform::get_current_dir()
{
	CHAR path[MAX_PATH + 1] = "";
	DWORD len = GetCurrentDirectoryA(MAX_PATH, path);

	UNUSED(len);

	return path;
}

} // lax