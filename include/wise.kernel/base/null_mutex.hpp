#pragma once 

namespace wise {
	namespace kernel {
		struct null_mutex
		{
			null_mutex() = default;

			int locked = 0;

			void lock() { locked++; }
			void unlock() noexcept { locked--; }
			bool try_lock() { 
				lock();  
				return true; 
			}
		};
	}
} // wise::kernel
