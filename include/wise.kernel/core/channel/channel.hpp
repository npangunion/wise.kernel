#pragma once 

#include <wise.kernel/core/channel/message.hpp>
#include <wise.kernel/core/channel/sub_map.hpp>

#include <wise.kernel/core/tick.hpp>
#include <wise.kernel/core/mem_tracker.hpp>
#include <wise.kernel/core/concurrent_queue.hpp>
#include <atomic>
#include <functional>

namespace wise {
namespace kernel {

using skey_t = sub::key_t;

/// �޼��� ���� ä�� 
/**
 * channel�� �޼����� �����ϴ� ��θ� ����
 *  topic(uint32_t)�� lambda �Լ� (std::function)�� ���
 *  ä�ΰ� ���ᵵ �Լ��� ����Ѵ�.
 *  ������ Ű ��ü, Ÿ�ٿ� ���ؼ��� subscribe �� �� �ִ�.
 *  �������� �� Ű ��ü, Ÿ�ٿ� ���ؼ��� ������ �õ��Ѵ�.
 *
 * ����:
 *  channel::create(name)
 *
 * ���:
 *  auto ch = channel::find( "key" );
 *  auto skey = ch->sub();
 *  ch->unsub(skey);
 *
 * ��������:
 *  ä���� �����ϴ� ������ post() �ֱ����� ȣ��
 *
 * �Ҹ�:
 *  channel::destroy(name)
 *
 * ���(sub):
 *   ���� ������ ����
 *   topic - �ް��� �ϴ� ���� (����)
 *   cond - ���� üũ (���� ���͸�)
 *   cb - �ݹ�
 *
 * ���� (post) ���:
 *   ���(immediate) ���� publish()�� �� ����.
 *   ����(delayed) ���� post() ȣ�� �� ����
 *
 */
class channel
{
public:
	using ptr = std::shared_ptr<channel>;
	using key_t = sub::channel_key_t;
	using cond_t = sub::cond_t;
	using cb_t = sub::cb_t;
	using mq_t = concurrent_queue<message::ptr>;

	/// config 
	struct config
	{
		/// limit post count in a post loop
		std::size_t loop_post_limit = UINT32_MAX;

		/// ���� �޼��� ���� ���� �ð��� �� �ð��� ���� ��� �α� ����
		tick_t time_to_log_when_post_time_over = 20;

		/// post() ������ ��ü ���� ���� �ð��� �� �ð��� ���� ��� �α� ����
		tick_t time_to_log_when_post_loop_time_over = 200;

		/// ���ȿ� ��ϵ� �ݹ��� ���� ��� �α� ����
		bool log_no_sub_when_post = false;

		/// log remaining messages in queue on exit
		bool log_remaining_messages_on_exit = false;

		/// �Ҹ��ڿ��� ������ ��ü ó���ϰ� ������ �� ����
		// �������� �� �Ͽ� �ɼ��� ����
		// bool post_all_when_destroyed = true;

		config() = default;

		config(
			std::size_t lpl,
			tick_t ttl_post_over,
			tick_t ttl_loop_over,
			bool log_no_sub
		)
			: loop_post_limit(lpl)
			, time_to_log_when_post_time_over(ttl_post_over)
			, time_to_log_when_post_loop_time_over(ttl_loop_over)
			, log_no_sub_when_post(log_no_sub)
		{
		}
	};

	/// thread-unsafe stat block
	struct stat
	{
		std::size_t post_time_over_count = 0;
		std::size_t post_loop_time_over_count = 0;

		std::size_t total_post_count = 0;
		std::size_t total_immediate_post_count = 0;
		std::size_t last_post_count = 0;
	};

public:
	/// create a channel and register it to a map
	/**
	 * @return ������ ä��. �ߺ��Ǹ� ������ channel::ptr() ����.
	 */
	static ptr create(const key_t& key, const config& cfg);

	/// find a channel with a key
	static ptr find(const key_t& key);

	/// find a channel from a address key
	static ptr find(uintptr_t pkey);

	/// destroy a channel
	static bool destroy(const key_t& key);


public:
	channel(const key_t& key)
		: channel(key, config())
	{
	}

	/// ������ 
	/**
	 * key�� ���� ������ ����
	 *
	 * @param key - ä���� ���� �̸�
	 * @param cfg - ���� ����
	 */
	channel(const key_t& key, const config& cfg);

	/// �Ҹ���
	~channel();

	/// publish 
	/**
	 * �޼����� �������ε� ������.
	 * ť�� ���� �ְ� �������� ����.
	 * @param m - message
	 *
	 * @return number of dispatched count
	 */
	std::size_t publish(message::ptr m);

	/// publish with a separate topic from message's topic
	/**
	 * �ܺο��� �����ϴ� �������� ������.
	 * ���� �޼����� �������ε� ������.
	 * ť�� ���� �ְ� �������� ����.
	 *
	 * @param topic - topic to dispatch first
	 * @param m - message
	 *
	 * dispatch with a topic first. then, call publish(m) inside.
	 *
	 * @return number of dispatched cont
	 */
	std::size_t publish(const topic& topic, message::ptr m);

	/// post all messages in queue
	/**
	 * sub::mode::delayed subscriptions are handled for messages
	 */
	std::size_t execute();

	/// subscribe to a topic with a condition 
	/**
	 * @param topic - a topic to subscribe
	 * @param cond - condition to check before dispatch
	 * @param cb - callback function
	 * @param mode - dispatch mode
	 *
	 * @return the key to the subscription
	 */
	sub::key_t subscribe(
		const topic& topic,
		cond_t cond,
		cb_t cb,
		sub::mode mode = sub::mode::delayed
	);

	/// subscribe to a topic without condition 
	/**
	 * @param topic - a topic to subscribe
	 * @param cb - callback function
	 *
	 * @return the key to the subscription
	 */
	sub::key_t subscribe(
		const topic& topic,
		cb_t cb,
		sub::mode mode = sub::mode::delayed
	);

	/// unsubscribe 
	/**
	 * @param key - a subscription key
	 *
	 * @return ��ϵ� �� ����� true, ������ false
	 */
	bool unsubscribe(sub::key_t key);

	/// clear all subscriptions
	void clear();

	const config& get_config() const
	{
		return config_;
	}

	/// Ű�� ������
	const key_t& get_key() const
	{
		return key_;
	}

	uintptr_t get_pkey() const
	{
		return reinterpret_cast<uintptr_t>(this);
	}

	/// queue size (unsafe)
	std::size_t get_queue_size() const;

	/// stat
	const stat& get_stat() const
	{
		return stat_;
	}

private:
	key_t   key_;
	mq_t	q_;
	sub_map map_;
	config  config_;
	stat	stat_;
};

} // kernel
} // wise
