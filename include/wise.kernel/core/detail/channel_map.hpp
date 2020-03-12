#pragma once 

#include <wise.kernel/core/index.hpp>
#include <wise.kernel/core/channel.hpp>
#include <map>

namespace wise {
	namespace kernel {

		class channel_map
		{
		public:
			static channel_map& get();

			~channel_map();

			channel::ptr create(const channel::key_t& key, const channel::config& cfg);

			channel::ptr find(const channel::key_t& key);

			channel::ptr find_from_addr(uintptr_t pkey);

			bool destroy(const channel::key_t& key);

		private:
			using map = std::map<channel::key_t, channel::ptr>;
			using pindex = index<uintptr_t, channel::key_t>;

			channel_map();

		private:
			std::shared_timed_mutex		mutex_;
			map							map_;
			pindex						pkey_index_;
		};

	} // kernel
} // wise