#pragma once 

#include <wise.kernel/core/detail/sub.hpp>
#include <wise.kernel/core/sequence.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/concurrent_queue.hpp>
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
		 * thread-safe using shared_timed_mutex.
		 */
		class sub_map
		{
		public:
			/// ������
			sub_map(channel& _channel, const std::string& desc);

			/// �Ҹ���
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

			/// topic�� ���� ���� ó���� �׸��� �ִ� �� Ȯ��
			bool has_delayed_sub(const topic& topic) const;

			/// �ϳ��� �ִ� �� Ȯ��
			bool has_delayed_sub() const;

			/// ��ü subscription ����
			std::size_t get_subscription_count() const;

			/// ����� ��. topic�� ��ϵ� ����
			std::size_t get_subscription_count(const topic& topic) const;

			/// ��庰 ��ϵ� ����
			std::size_t get_subscription_count(const topic& topic, sub::mode mode) const;

		private:
			struct entry
			{
				topic				topic;
				std::size_t			post_count = 0;
				std::vector<sub>	subs;
			};
			using entry_map = std::unordered_map<topic, entry>;

			struct entry_link
			{
				topic				topic;
				sub::mode			mode;
			};
			using key_map = std::unordered_map<sub::key_t, entry_link>;

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

			void process_pending_unsubs();

			uint64_t get_current_thread_hash() const;

		private:
			channel& channel_;
			std::string							desc_;
			mutable std::shared_timed_mutex		mutex_;
			entry_map							entries_immediate_;
			entry_map							entries_delayed_;
			key_map								keys_;
			sequence<sub::key_t>				seq_;
			std::atomic<bool>					is_posting_ = false;
			std::atomic<uint64_t>				posting_thread_ = 0;
			concurrent_queue<sub::key_t>		pending_unsubs_;
		};

	} // kernel
} // wise
