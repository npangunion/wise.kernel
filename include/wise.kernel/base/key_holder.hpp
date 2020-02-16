#pragma once 

#include <wise/channel/channel.hpp>

namespace wise
{

template <typename Owner, typename Key>
class key_holder
{
public: 
	key_holder()
	{
	}

	~key_holder()
	{
		for (auto& key : keys_)
		{
			channel_->unsubscribe(key);
		}
	}

	void subscribe(
		const topic& topic,
		channel::cond_t cond,
		channel::cb_t cb,
		sub::mode mode = sub::mode::delayed
	)
	{
		auto key = channel_->subscribe(topic, cond, cb, mode);
		keys_.push_back(key);
	}

	void subscribe(
		const topic& topic,
		channel::cb_t cb,
		sub::mode mode = sub::mode::delayed
	)
	{
		auto key = channel_->subscribe(topic, cb, mode);
		keys_.push_back(key);
	}

private: 
	channel::ptr channel_;
	std::vector<sub::key_t> keys_;
};

} // wise