#pragma once 

#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace wise {
	namespace kernel {

		/// unique lock with optional flag. 
		class use_lock_unique
		{
		public:
			use_lock_unique(bool use, std::shared_timed_mutex& mtx)
				: use_(use)
				, mutex_(mtx)
			{
				if (use_)
				{
					mutex_.lock();
				}
			}

			~use_lock_unique()
			{
				if (use_)
				{
					mutex_.unlock();
				}
			}

		private:
			std::atomic<bool> use_;
			std::shared_timed_mutex& mutex_;
		};

		/// shared lock with optional flag. 
		class use_lock_shared
		{
		public:
			use_lock_shared(bool use, std::shared_timed_mutex& mtx)
				: use_(use)
				, mutex_(mtx)
			{
				if (use_)
				{
					mutex_.lock_shared();
				}
			}

			~use_lock_shared()
			{
				if (use_)
				{
					mutex_.unlock_shared();
				}
			}

		private:
			std::atomic<bool> use_;
			std::shared_timed_mutex& mutex_;
		};

	} // kernel
} // wise
