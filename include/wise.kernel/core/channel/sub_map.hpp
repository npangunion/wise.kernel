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
	sub_map(channel& _channel, const std::string& desc);

	/// 소멸자
	~sub_map();

	/// subscribe to topic. locked with unique_lock
	sub::key_t subscribe(
		const topic& topic,
		sub::cond_t& cond,
		sub::cb_t& cb,
		sub::mode mode
	);

	/// subscribe to topic. locked with unique_lock
	sub::key_t subscribe(const topic& topic, sub::cb_t& cb, sub::mode mode);

	/// unsubscribe to topic. locked with unique_lock
	bool unsubscribe(sub::key_t key);

	/// post a topic and message topic. locked with shared_lock
	std::size_t post(const topic& topic, message::ptr m, sub::mode mode);

	/// post a message topic. locked with shared_lock
	std::size_t post(message::ptr m, sub::mode mode);

	/// clear subscriptions
	void clear();

	/// topic에 대해 나중 처리할 항목이 있는 지 확인
	bool has_delayed_sub(const topic& topic) const;

	/// 하나라도 있는 지 확인
	bool has_delayed_sub() const;

	/// 전체 subscription 개수
	std::size_t get_subscription_count() const;

	/// 디버깅 용. topic에 등록된 개수
	std::size_t get_subscription_count(const topic& topic) const;

	/// 모드별 등록된 개수
	std::size_t get_subscription_count(const topic& topic, sub::mode mode) const;

private:
	struct entry
	{
		topic				topic_;
		std::size_t			post_count_ = 0;
		std::vector<sub>	subs_;
	};

	using entry_map = std::unordered_map<topic, entry>;

	struct entry_link
	{
		topic				topic;
		sub::mode			mode;
	};
	using key_map = std::unordered_map<sub::key_t, entry_link>;

	const int purge_sub_interval = 5000;

private:
	sub::key_t subscribe(
		entry_map& em,
		const topic& topic,
		sub::cond_t& cond,
		sub::cb_t& cb,
		sub::mode mode);

	sub::key_t subscribe(
		entry& e,
		const topic& topic,
		sub::cond_t& cond,
		sub::cb_t& cb,
		sub::mode mode
	);

	bool unsubscribe(entry_map& em, sub::key_t key);

	std::size_t post(entry_map& em, const topic& topic, message::ptr m);

	std::size_t post(entry_map& em, message::ptr m);

	std::size_t post_on_topic(entry_map& em, const topic& topic, message::ptr m);

	void purge_unsubscribed_entries();

	void purge_unsubscribed(entry_map& m);

	uint64_t get_current_thread_hash() const;

private:
	channel& channel_;
	std::string							desc_;
	mutable std::shared_mutex			mutex_;
	entry_map							entries_immediate_;
	entry_map							entries_delayed_;
	key_map								keys_;
	sequence<sub::key_t>				seq_;
	std::atomic<uint64_t>				posting_thread_ = 0;
	simple_tick							purge_tick_;
};

} // kernel
} // wise
