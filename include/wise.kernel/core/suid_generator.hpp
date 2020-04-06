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

/// 40 bit ms | 12 bit domain | 12 bit sequence
/**
 * snowflake �˰����� �����Ͽ� ����
 * 1ms�� 4096������ ������ ���̵� ����. 
 * 1ms �̳��� 4096�� ��û�� ���� ��� sleep���� ���
 * �̿� ���� ���� ������ ���� ��쿡�� ��� ����
 */
template <typename Mutex = null_mutex>
class suid_generator
{
public:
	/// 12bit domain���� ���еǴ� ���� ���̵� ����
	suid_generator()
		: domain_(0)
	{
	}

	void setup(uint16_t domain)
	{
		domain_ = domain;
		WISE_THROW_IF(domain_ == 0, "domain_ must be positive number");
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
				seq_ = (seq_ + 1) & 0x0FFF;

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

			now_id = ((now_ms & 0xFFFFFFFFFF) << 24) | ((domain_ & 0x0FFF) << 12) | (seq_ & 0x0FFFF);
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
		// �и��ʴ� 4096���� ������ ���� �� ���� ������. 
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
	uint16_t seq_;
	suid_t last_id_ = 0;
	int sleep_count_ = 0;
};

} // kernel
} // wise 