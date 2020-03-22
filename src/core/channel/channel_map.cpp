#include <pch.hpp>
#include <wise.kernel/core/channel/channel_map.hpp>
#include <wise.kernel/core/mem_tracker.hpp>

namespace wise {
namespace kernel {

channel_map& channel_map::get()
{
	static channel_map inst;

	return inst;
}

channel_map::channel_map()
{
}

channel_map::~channel_map()
{
	map_.clear();
}

channel::ptr channel_map::create(const channel::key_t& key, const channel::config& cfg)
{
	// shared lock. WISE_ASSERT
	{
		std::shared_lock<std::shared_timed_mutex> lock(mutex_);

		auto iter = map_.find(key);
		WISE_ASSERT(iter == map_.end());

		// 중복되면 에러로 처리
		WISE_RETURN_IF(iter != map_.end(), channel::ptr());
	}

	auto cp = wise_shared<channel>(key, cfg);

	// exclusive lock. insert
	{
		std::unique_lock<std::shared_timed_mutex> lock(mutex_);

		map_.insert(map::value_type(key, cp));
		pkey_index_.set(cp->get_pkey(), key);
	}

	WISE_ASSERT(pkey_index_.size() == map_.size());

	return cp;
}

channel::ptr channel_map::find(const channel::key_t& key)
{
	std::shared_lock<std::shared_timed_mutex> lock(mutex_);

	auto iter = map_.find(key);
	WISE_RETURN_IF(iter == map_.end(), channel::ptr());

	return iter->second;
}

channel::ptr channel_map::find_from_addr(uintptr_t pkey)
{
	channel::key_t key;

	// lock block
	{
		std::shared_lock<std::shared_timed_mutex> lock(mutex_);
		WISE_RETURN_IF(!pkey_index_.has(pkey), channel::ptr());

		key = pkey_index_.get(pkey);
	}

	return find(key);
}

bool channel_map::destroy(const channel::key_t& key)
{
	std::unique_lock<std::shared_timed_mutex> lock(mutex_);

	auto iter = map_.find(key);
	WISE_ASSERT(iter != map_.end());
	WISE_RETURN_IF(iter == map_.end(), false);

	pkey_index_.unset(iter->second->get_pkey());
	map_.erase(iter);

	WISE_ASSERT(pkey_index_.size() == map_.size());

	return true;
}

} // kernel
} // wise
