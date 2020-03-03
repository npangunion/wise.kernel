#pragma once 

namespace wise {
	namespace kernel {
		struct null_mutex
		{
			null_mutex() = default;

			void lock() {}
			void unlock() noexcept {}
			bool try_lock() { return true; }
		};
	}
} // wise::kernel
