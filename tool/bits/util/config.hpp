#pragma once 

#define R2C_IGNORE_MACRO_INSUFFICIENT_ARGUMENT 1

#if R2C_IGNORE_MACRO_INSUFFICIENT_ARGUMENT
	#pragma warning(disable: 4003)	// macro 인수 부족 경고. 편하게 쓰기 위해 위반.
#endif

#define R2C_ENABLE_CHECK_LOG		1	// check 위반 사항을 로그로 남김 
#define R2C_ENABLE_RELEASE_CHECK	0	// release 모드에서 check 처리 

#ifdef _MSC_VER 
#define R2C_ENABLE_CONCURRENT_QUEUE 1
#else 
#define R2C_ENABLE_CONCURRENT_QUEUE 0
#endif
