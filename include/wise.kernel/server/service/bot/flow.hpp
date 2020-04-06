#pragma once

#include <wise/service/bot/act.hpp>
#include <wise/base/index.hpp>
#include <memory>

namespace wise
{

class agent;

/// has acts and runs them
class flow
{
public:
	using config = nlohmann::json;
	using ptr = std::shared_ptr<flow>;
	using key = std::string;

public: 
	explicit flow(agent& owner, const std::string& name, config& cfg);
	~flow();

	bool start();

	void exec();

	void stop();

	act::ptr find(const act::key& key);

	void end_current();

	agent& get_owner()
	{
		return owner_;
	}

	const agent& get_owner() const
	{
		return owner_;
	}

	const std::string& get_key() const
	{
		return key_;
	}

	std::size_t size() const
	{
		return acts_.size();
	}

	bool is_active() const
	{
		return active_;
	}

	act::ptr get_act(const act::key& kv)
	{
		if (key_index_.has(kv))
		{
			auto ap = acts_[key_index_.get(kv)];
			WISE_ASSERT(ap->get_index() == key_index_.get(kv));

			return ap;
		}

		return act::ptr();
	}

	act::ptr get_act(std::size_t index)
	{
		if (index < acts_.size())
		{
			return acts_[index];
		}

		return act::ptr();
	}

	bool has_act(const act::key& key)
	{
		return !!get_act(key);
	}

	bool has_active_acts() const
	{
		for (auto& ap : acts_)
		{
			if (ap->is_active())
			{
				return true;
			}
		}

		return false;
	}

private:
	using acts = std::vector<act::ptr>;
	using key_index = index<std::string, std::size_t>;

	void load(const config& cfg);
	void load_act_push(act::ptr ap);

private: 
	agent&	owner_;
	std::string key_;
	config& cfg_;
	acts acts_;
	key_index key_index_;
	act::ptr act_0_;
	bool active_;
};

} // wise
