#pragma once 

#include <wise.kernel/core/null_mutex.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/exception.hpp>
#include <wise.kernel/util/util.hpp>
#include <chrono>
#include <mutex>

namespace wise {
namespace kernel {

using suid_t = uint64_t;

/// 40 bit ms | 16 bit domain | 8 bit sequence
/**
 * 16비트 도메인을 서버 아이디로 지정. 
 * snowflake 알고리즘을 수정하여 구현
 * 1ms에 256개 고유한 아이디를 생성. 
 * 1ms 이내에 256개 요청이 오면 잠시 sleep으로 대기
 * 40bit면 거의 영원히 사용 가능.
 * 잦은 생성이 없는 경우에 매우 오래 사용하는 고유 아이디
 */
template <typename Mutex = null_mutex>
class actor_id_generator
{
public:
	/// 12bit domain으로 구분되는 고유 아이디를 생성
	actor_id_generator()
		: domain_(0)
	{
	}

	void setup(uint16_t domain)
	{
		domain_ = domain;
		WISE_THROW_IF(domain_ == 0, "domain_ must be positive number");
		WISE_THROW_IF(domain_ >= 0xFFFF, "domain_ number must be less than 0xFFFF");
	}

	/// roughly following snow-flake algorithm implementation
	suid_t next()
	{
		WISE_THROW_IF(domain_ == 0, "domain_ must be positive number");

		auto now_ms = get_ms_since_epoch();

		bool log_sleep = false;

		suid_t now_id = 0;

		// locked
		{
			std::lock_guard<Mutex> lock(mutex_);

			if (now_ms < last_ms_)
			{
				// can happen? really? 
				WISE_THROW_FMT(
					"time inverted. now: {}, last: {}", now_ms, last_ms_
					);
			}

			if (last_ms_ == now_ms)
			{
				seq_ = (seq_ + 1) & 0xFF;

				if (seq_ == 0)
				{
					now_ms = wait_next_ms();
					log_sleep = true;
					WISE_ASSERT(now_ms > last_ms_);
				}
			}
			else
			{
				seq_ = 0;
			}

			uint64_t ms = (now_ms & 0xFFFFFFFFFF) << 24;
			uint64_t dm = (domain_ & 0xFFFF) << 8;
			uint64_t sp = (seq_ & 0xFF);
			now_id = (ms | dm | sp);
			WISE_ASSERT(now_id != last_id_);

			last_ms_ = now_ms;
			last_id_ = now_id;
		}

		if (log_sleep)
		{
			WISE_WARN("suid generator slept: {}", sleep_count_);
		}

		return now_id;
	}

	uint16_t get_domain_of(suid_t id) const
	{
		return (id >> 8) & 0x000000000000FFFF;
	}

	uint16_t get_domain() const
	{
		return domain_;
	}

private:
	uint64_t get_ms_since_epoch()
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
			);

		return ms.count();
	}

	uint64_t wait_next_ms()
	{
		// 밀리초당 4096개가 넘으면 재우는 게 가장 빠르다. 
		sleep(1);

		++sleep_count_;

		return get_ms_since_epoch();
	}

	int get_sleep_count() const
	{
		return sleep_count_;
	}

private:
	uint16_t domain_;
	Mutex mutex_;
	uint64_t last_ms_ = 0;
	uint16_t seq_ = 0;
	suid_t last_id_ = 0;
	int sleep_count_ = 0;
};

} // kernel
} // wise 