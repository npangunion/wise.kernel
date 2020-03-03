#pragma once 

#define WISE_IGNORE_MACRO_INSUFFICIENT_ARGUMENT 1

#if WISE_IGNORE_MACRO_INSUFFICIENT_ARGUMENT
	#ifdef _MSC_VER
		#pragma warning(disable: 4003)	// macro 인수 부족 경고. WISE_RETURN_IF 편하게 쓰기 위해 위반.
	#endif
#endif

#define WISE_ENABLE_CHECK_LOG		1	// check 위반 사항을 로그로 남김 
#define WISE_ENABLE_RELEASE_CHECK	1	// release 모드에서 check 처리 
#define WISE_ENABLE_ASSERT_FAIL_LOG 1	// ensure / expect / WISE_ASSERT 실패 시 로깅 여부  

#ifdef _MSC_VER 
	#define WISE_ENABLE_CONCURRENT_QUEUE 1
#else 
	#define WISE_ENABLE_CONCURRENT_QUEUE 0
#endif

#define WISE_ENABLE_LOG_TRACE		0	// enable function@line trace appended

#define WISE_USE_STACKTRACE			1	// enable stacktrace

#ifdef _MSC_VER
#define WISE_TRACK_MEMORY			1	// trace memory usage 
#else 
#define WISE_TRACK_MEMORY			0	// trace memory usage 
#endif

#if WISE_TRACK_MEMORY 
#define WISE_TRACK_GLOBAL_MEMORY	1
#endif

#define WISE_USE_JEMALLOC			0	// use jemalloc for global memory allocation


// spinlock
#define WISE_FIBERS_CONTENTION_WINDOW_THRESHOLD 16
#define WISE_FIBERS_RETRY_THRESHOLD 64
#define WISE_FIBERS_SPIN_BEFORE_SLEEP0 256
#define WISE_FIBERS_SPIN_BEFORE_YIELD 128

