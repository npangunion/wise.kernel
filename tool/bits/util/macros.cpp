#include "stdafx.h"

#include "macros.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace r2c
{
/// cond에 따라 로그를 남긴다. 
void check_(bool cond, const char* msg, const char* func, const char* file, int line)
{
	assert(!cond);
	UNUSED(cond);

	std::ostringstream oss;
	oss << " failed: " << msg << " [" << func << "]" << "[" << file << "@" << line << "]";

	std::ofstream f("check.log", std::ofstream::app);

	f << oss.str() << std::endl;

	f.close();
}
} // r2c
