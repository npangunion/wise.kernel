#include <pch.hpp>
#include <wise.kernel/core/channel/sub_map.hpp>
#include <wise.kernel/core/channel/channel.hpp>
#include <algorithm>

namespace wise {
namespace kernel {

sub_map::sub_map(channel& _channel)
	: seq_(1, UINT32_MAX, 1000)
	, channel_(_channel)
{
}

sub_map::~sub_map()
{
	clear();
}

sub::key_t sub_map::subscribe(const topic& topic, sub::cond_t cond, sub::cb_t cb, sub::mode mode)
{
	std::unique_lock<std::shared_mutex> lock(mutex_);

	auto key = seq_.next();

	sub s(key, topic, cond, cb, mode);
	all_subs_.insert(subs::value_type(key, s));

	if (mode == sub::mode::immediate)
	{
		subscribe_mode(entries_immediate_, s);
	}
	else
	{
		subscribe_mode(entries_delayed_, s);
	}

	return key;
}

sub::key_t sub_map::subscribe(const topic& topic, sub::cb_t cb, sub::mode mode)
{
	sub::cond_t cond = [](message::ptr m) { return true; };

	return subscribe(
		topic,
		cond,
		cb,
		mode
	);
}

bool sub_map::unsubscribe(sub::key_t key)
{
	std::unique_lock<std::shared_mutex> lock(mutex_);

	auto iter = all_subs_.find(key);
	WISE_RETURN_IF(iter == all_subs_.end(), false);

	iter->second.unsubscribe();
	purge_keys_.push_back(key);

	if (iter->second.get_mode() == sub::mode::immediate)
	{
		return unsubscribe_mode(entries_immediate_, iter->second);
	}
	else
	{
		return unsubscribe_mode(entries_delayed_, iter->second);
	}
}

sub& sub_map::get_sub(sub::key_t key)
{
	std::shared_lock<std::shared_mutex> lock(mutex_);

	auto iter = all_subs_.find(key);

	if (iter != all_subs_.end())
	{
		return iter->second;
	}

	WISE_THROW("subscription key is invalid");
}

void sub_map::clear()
{
	std::unique_lock<std::shared_mutex> lock(mutex_);

	entries_delayed_.clear();
	entries_immediate_.clear();
	all_subs_.clear();
}

void sub_map::subscribe_mode(entry_map& em, sub& s)
{
	auto iter = em.find(s.get_topic());

	if (iter == em.end())
	{
		topic_entry e;

		e.topic_ = s.get_topic();

		subscribe_topic(e, s);
		em.insert(entry_map::value_type(s.get_topic(), e));
	}
	else
	{
		subscribe_topic(iter->second, s);
	}
}

void sub_map::subscribe_topic(topic_entry& e, sub& s)
{
	e.subs_.insert(subs::value_type(s.get_key(), s));
}

bool sub_map::unsubscribe_mode(entry_map& em, sub& s)
{
	auto topic = s.get_topic();

	auto ei = em.find(topic);
	WISE_ASSERT(ei != em.end());
	WISE_RETURN_IF(ei == em.end(), false);

	auto& subs = ei->second.subs_;

	auto iter = subs.find(s.get_key());
	WISE_RETURN_IF(iter == subs.end(), false);

	iter->second.unsubscribe();

	return true;
}

std::size_t sub_map::post(const topic& topic, message::ptr m, sub::mode mode)
{
	std::size_t count = 0;

	// posting
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);

		if (mode == sub::mode::immediate)
		{
			count = post_mode(entries_immediate_, topic, m);
		}
		else
		{
			count = post_mode(entries_delayed_, topic, m);
		}
	}

	purge_wait();

	return count;
}

std::size_t sub_map::post(message::ptr m, sub::mode mode)
{
	std::size_t count = 0;

	// posting
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);

		if (mode == sub::mode::immediate)
		{
			count = post_mode(entries_immediate_, m);
		}
		else
		{
			count = post_mode(entries_delayed_, m);
		}
	}

	purge_wait();

	return count;
}

std::size_t sub_map::post_mode(entry_map& em, const topic& topic, message::ptr m)
{
	std::size_t count = 0;

	count += post_topic(em, topic, m);
	count += post_topic(em, topic.get_group_topic(), m);

	return count;
}

std::size_t sub_map::post_mode(entry_map& em, message::ptr m)
{
	std::size_t count = 0;

	count += post_topic(em, m->get_topic(), m);
	count += post_topic(em, m->get_topic().get_group_topic(), m);

	return count;
}

std::size_t sub_map::post_topic(entry_map& em, const topic& topic, message::ptr m)
{
	WISE_RETURN_IF(!topic.is_valid(), 0);

	auto iter = em.find(topic);

	if (iter == em.end())
	{
		if (channel_.get_config().log_no_sub_when_post)
		{
			WISE_WARN("{} has no subscription. channel:{}", channel_.get_key());
		}
		return 0;
	}

	auto& subs = iter->second.subs_;

	int count = 0;
	auto total = subs.size();

	WISE_ASSERT(total > 0);

	try
	{
		for (auto& kv : subs)
		{
			auto& s = kv.second;

			if (!s.is_unsubscribed() && s.post(m))
			{
				++count;

				WISE_NONE(
					"posted from channel: {}, msg: {}, posting index: {} of: {}",
					channel_.get_key(), m->get_desc(), count, total
				);
			}
		}
	}
	catch (std::exception & ex)
	{
		WISE_ERROR(
			"exception {} while posting at {}. index: {}",
			ex.what(), __FUNCTION__, count
		);
	}

	return count;
}

void sub_map::purge_wait()
{
	if (purge_tick_.elapsed() < purge_sub_interval)
	{
		return;
	}

	purge_tick_.reset();

	purge();
}

void sub_map::purge()
{
	std::unique_lock<std::shared_mutex> lock(mutex_);

	while (!purge_keys_.empty())
	{
		auto key = purge_keys_.front();

		auto iter = all_subs_.find(key);

		if (iter != all_subs_.end())
		{
			auto& s = iter->second;

			purge_mode(entries_immediate_, s);
			purge_mode(entries_delayed_, s);

			seq_.release(key);

			all_subs_.erase(iter);
		}

		purge_keys_.pop_front();
	}
}

void sub_map::purge_mode(entry_map& em, sub& s)
{
	auto iter = em.find(s.get_topic());

	if (iter != em.end())
	{
		iter->second.subs_.erase(s.get_key());
	}
}

} // kernel
} // wise
