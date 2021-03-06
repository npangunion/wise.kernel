﻿#pragma once 

#include <wise.kernel/core/config.hpp>
#include <cassert>

namespace wise {
namespace kernel {

void check__(bool cond, const char* msg, const char* func, const char* file, int line);

} // kernel
} // wise

#ifdef _DEBUG
	#define WISE_ASSERT(c) (void)(!!(c) || (wise::kernel::check__(!!(c), #c, __FUNCTION__, __FILE__, __LINE__), 0)); assert(c);
#else 
	#if WISE_ENABLE_RELEASE_CHECK  == 0
		#define WISE_ASSERT(c)  
	#else 
		#define WISE_ASSERT(c) (void)(!!(c) || (wise::kernel::check__(!!(c), #c, __FUNCTION__, __FILE__, __LINE__), 0))
	#endif
#endif 

/// precondition
#define WISE_EXPECT(c) WISE_ASSERT(c)		 
/// post condition
#define WISE_ENSURE(c) WISE_ASSERT(c)

#define WISE_RETURN_IF(c, v) if ((c)) return v;
#define WISE_RETURN_CALL_IF(c, v, func) if ((c)) { func(); return (v); }

#define WISE_UNUSED(v) static_cast<void>(v)

