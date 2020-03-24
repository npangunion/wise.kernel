#pragma once 

#include <wise.kernel/core/channel/sub.hpp>
#include <wise.kernel/core/sequence.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/concurrent_queue.hpp>
#include <wise.kernel/core/tick.hpp>
#include <unordered_map>
#include <shared_mutex>
#include <vector>

namespace wise {
namespace kernel {

class channel;

/// subscriptions map
/**
 * used internally from channel class.
 *
 * thread-safe using shared_mutex.
 */
class sub_map
{
public:
	/// 생성자
	sub_map(channel& _channel);

	/// 소멸자
	~sub_map();

	/// subscribe to topic. locked with unique_lock
	sub::key_t subscribe(
		const topic& topic,
		sub::cond_t cond,
		sub::cb_t cb,
		sub::mode mode);

	/// subscribe to topic. locked with unique_lock
	sub::key_t subscribe(
		const topic& topic,
		sub::cb_t cb,
		sub::mode mode);

	/// unsubscribe to topic. locked with unique_lock
	bool unsubscribe(sub::key_t key);

	/// post a topic and message topic. locked with shared_lock
	std::size_t post(const topic& topic, message::ptr m, sub::mode mode);

	/// post a message topic. locked with shared_lock
	std::size_t post(message::ptr m, sub::mode mode);

	/// find for test
	sub& get_sub(sub::key_t key);

	/// clear subscriptions
	void clear();

private:
	using subs = std::unordered_map<sub::key_t, sub>;

	struct topic_entry
	{
		topic				topic_;
		std::size_t			post_count_ = 0;
		subs				subs_;
	};

	using entry_map = std::unordered_map<topic, topic_entry>;

	const int purge_sub_interval = 5000;

private:
	void subscribe_mode(entry_map& em, sub& s);

	void subscribe_topic(topic_entry& e, sub& s);

	bool unsubscribe_mode(entry_map& em, sub& s);

	std::size_t post_mode(entry_map& em, const topic& topic, message::ptr m);

	std::size_t post_mode(entry_map& em, message::ptr m);

	std::size_t post_topic(entry_map& em, const topic& topic, message::ptr m);

	void purge_wait();

	void purge();

	void purge_mode(entry_map& em, sub& s);

private:
	channel&							channel_;
	mutable std::shared_mutex			mutex_;
	entry_map							entries_immediate_;
	entry_map							entries_delayed_;
	subs								all_subs_;
	sequence<sub::key_t>				seq_;
	simple_tick							purge_tick_;
	std::deque<sub::key_t>				purge_keys_;
};

} // kernel
} // wise
