#pragma once

#include <wise.kernel/server/actor.hpp>
#include <shared_mutex>

namespace wise
{
namespace kernel
{

class actor_directory
{
public:
	actor_directory();

	~actor_directory();

	bool add(actor::ref r)
	{
		WISE_THROW_IF(has(r->get_id()), fmt::format("duplicate id for actor. id:{}", ap->get_id()).c_str());

		std::unique_lock<std::shared_mutex> ul(lock_);
		actors_.insert(actor_map::value_type(ap->get_id(), ap));

		return true;
	}

	bool has(actor::id_t id) const
	{
		std::shared_lock<std::shared_mutex> sl(lock_);
		return actors_.find(id) != actors_.end();
	}

	actor::ref get(actor::id_t id)
	{
		std::shared_lock<std::shared_mutex> sl(lock_);

		auto iter = actors_.find(id);
		if (iter != actors_.end())
		{
			return actor::ref(iter->second);
		}

		return actor::ref();
	}

	void del(actor::ptr ap)
	{
		del(ap->get_id());
	}

	void del(actor::id_t id)
	{
		std::unique_lock<std::shared_mutex> ul(lock_);
		actors_.erase(id);
	}

private: 
	using actor_map = std::map < actor::id_t, actor::ptr>;

	void cleanup();

private:
	mutable std::shared_mutex lock_;
	actor_map actors_;
};

} // kernel
} // wise