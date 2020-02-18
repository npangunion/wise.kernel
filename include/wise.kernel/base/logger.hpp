#pragma once

#include <wise.kernel/base/config.hpp>
#include <wise.kernel/base/macros.hpp>
#include <memory>
#include <mutex>

namespace wise {
	namespace kernel {

		/// system logger. 
		class system_logger
		{
		public:
			static unsigned int async_queue_size;		// default 1024*1024. must be power of 2 
			static unsigned int flush_interval_seconds;	// default 2 seconds
			static const char* file_prefix;				// default log_ (ex. logs/system.log) 
			static const char* log_pattern;				// default [%H:%M:%S %z][%t]  
			static bool enable_console;					// default true. show console log
			static bool retry_on_overflow;				// default true. when log buffer overflows, retry log

		public:
			/// initialize only once. 실패하지 않아야 한다.
			static void init();

			static std::shared_ptr<spdlog::logger> get()
			{
				if (!initialized_)
				{
					// 한번만 초기화 하고 초기화 대기 위해 락 사용. 
					// 성능을 위해 미리 체크하고 진행

					init();
				}

				if (!logger_)
				{
					logger_ = spdlog::get(name);
				}

				return logger_;
			}

		private:
			static std::shared_ptr<spdlog::logger> logger_;	// to cache
			static std::recursive_mutex mutex_;
			static const char* name;					// default "system"

		private:
			static std::atomic<bool> initialized_;
		};

		inline std::shared_ptr<spdlog::logger> log()
		{
			return system_logger::get();
		}


	} 
} // wise::kernel

#define WISE_STR_H(x) #x
#define WISE_STR_HELPER(x) WISE_STR_H(x) 
#define WISE_NO_LOG(...) 

#if	WISE_ENABLE_LOG_TRACE == 1
#define WISE_NONE(...)			
#define WISE_TRACE(...)			wise::log()->trace(		"[" __FUNCTION__ "@" WISE_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#define WISE_DEBUG(...)			wise::log()->debug(		"[" __FUNCTION__ "@" WISE_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#define WISE_INFO(...)			wise::log()->info(		"[" __FUNCTION__ "@" WISE_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#define WISE_WARN(...)			wise::log()->warn(		"[" __FUNCTION__ "@" WISE_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#define WISE_ERROR(...)			wise::log()->error(		"[" __FUNCTION__ "@" WISE_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#define WISE_CRITICAL(...)		wise::log()->critical(	"[" __FUNCTION__ "@" WISE_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#else
#define WISE_NONE(...)			
#define WISE_TRACE(...)			wise::log()->trace(		__VA_ARGS__ )
#define WISE_DEBUG(...)			wise::log()->debug(		__VA_ARGS__	)
#define WISE_INFO(...)			wise::log()->info(		__VA_ARGS__	)
#define WISE_WARN(...)			wise::log()->warn(		__VA_ARGS__	)
#define WISE_ERROR(...)			wise::log()->error(		__VA_ARGS__	)
#define WISE_CRITICAL(...)		wise::log()->critical(	__VA_ARGS__	)
#endif
