#pragma once

#include <wise.kernel/server/actor.hpp>
#include <wise.kernel/core/index.hpp>
#include <shared_mutex>

namespace wise {
namespace kernel {

class actor_directory
{
public:
	actor_directory();

	~actor_directory();

	bool add(actor::ref aref)
	{
		WISE_THROW_IF(has(aref.get_id()), 
			fmt::format("duplicate id for actor. id:{:x}", 
				aref.get_id()).c_str()
		);

		std::unique_lock<std::shared_mutex> ul(lock_);
		actors_.insert(actor_map::value_type(aref.get_id(), aref));

		return true;
	}

	bool add(const std::string& name, actor::ref aref)
	{
		// check 
		{
			std::shared_lock<std::shared_mutex> ul(lock_);
			if (name_index_.has(name))
			{
				WISE_THROW_FMT("name: {} already exists in index", name);
			}
		}

		if (add(aref))
		{
			aref.set_name(name);

			std::unique_lock<std::shared_mutex> ul(lock_);
			name_index_.set(name, aref.get_id());

			return true;
		}

		return false;
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

	actor::ref get(const std::string& name)
	{
		actor::id_t id = 0;

		// get
		{
			std::shared_lock<std::shared_mutex> sl(lock_);
			id = name_index_.get(name);
		}

		return get(id);
	}

	void del(actor::id_t id)
	{
		auto aref = get(id);

		// unique locked
		{
			std::unique_lock<std::shared_mutex> ul(lock_);
			actors_.erase(id);

			if (aref.has_name())
			{
				name_index_.unset(aref.get_name());
			}
		}
	}

private: 
	using actor_map = std::map < actor::id_t, actor::ref>;
	using name_index = index<std::string, actor::id_t>;

	void cleanup();

private:
	mutable std::shared_mutex lock_;
	actor_map actors_;
	name_index name_index_;
};

} // kernel
} // wise