#pragma once 

#include "config.hpp"

#include <cassert>

namespace r2c
{
void check_(bool cond, const char* msg, const char* func, const char* file, int line);
}

#ifdef _DEBUG
	#define VERIFY(c) (void)(!!(c) || (r2c::check_(!!(c), #c, __FUNCTION__, __FILE__, __LINE__), 0)); assert(c);
#else 
	#if R2C_ENABLE_RELEASE_CHECK  == 0
		#define VERIFY(c) 
	#else 
		#define VERIFY(c) (void)(!!(c) || (r2c::check_(!!(c), #c, __FUNCTION__, __FILE__, __LINE__), 0))
	#endif
#endif 

/// precondition
#define EXPECT(c) VERIFY(c)		 
/// post condition
#define ENSURE(c) VERIFY(c)

#define RETURN_IF(c, v) if ((c)) return v
#define BREAK_IF(c, v) if ((c)) break 
#define CONTINUE_IF(c, v) if ((c)) continue 

#define UNUSED(v) v
