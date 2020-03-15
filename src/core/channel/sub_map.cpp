#include <pch.hpp>
#include <wise.kernel/core/channel/sub_map.hpp>
#include <wise.kernel/core/channel/channel.hpp>
#include <algorithm>

namespace wise {
	namespace kernel {

		sub_map::sub_map(channel& _channel, const std::string& desc)
			: seq_(1, UINT32_MAX, 1000)
			, channel_(_channel)
			, desc_(desc)
		{
		}

		sub_map::~sub_map()
		{

		}

		sub::key_t sub_map::subscribe(const topic& topic, sub::cond_t& cond, sub::cb_t& cb, sub::mode mode)
		{
			std::unique_lock<std::shared_timed_mutex> lock(mutex_);

			if (mode == sub::mode::immediate)
			{
				return subscribe(
					entries_immediate_,
					topic,
					cond,
					cb,
					mode);
			}
			else
			{
				return subscribe(
					entries_delayed_,
					topic,
					cond,
					cb,
					mode);
			}
		}

		sub::key_t sub_map::subscribe(const topic& topic, sub::cb_t& cb, sub::mode mode)
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
			if (is_posting_)
			{
				if (posting_thread_ == get_current_thread_hash())
				{
					WISE_THROW("posting thread cannot unsubscribe while posting.");
				}
			}

			std::unique_lock<std::shared_timed_mutex> lock(mutex_);

			// lock 이후에 있어야 한다. 
			WISE_THROW_IF(is_posting_, "unsubscribe during post is not allowed");

			auto iter = keys_.find(key);

			// this can happen after network finish
			// WISE_ASSERT(iter != keys_.end());

			WISE_RETURN_IF(iter == keys_.end(), false);

			if (iter->second.mode == sub::mode::immediate)
			{
				return unsubscribe(entries_immediate_, key);
			}
			else
			{
				return unsubscribe(entries_delayed_, key);
			}
		}

		void sub_map::clear()
		{
			std::unique_lock<std::shared_timed_mutex> lock(mutex_);

			entries_delayed_.clear();
			entries_immediate_.clear();
			keys_.clear();
		}

		bool sub_map::has_delayed_sub(const topic& topic) const
		{
			std::shared_lock<std::shared_timed_mutex> lock(mutex_);

			auto iter = entries_delayed_.find(topic);

			return iter != entries_delayed_.end();
		}

		bool sub_map::has_delayed_sub() const
		{
			std::shared_lock<std::shared_timed_mutex> lock(mutex_);
			return entries_delayed_.empty();
		}

		std::size_t sub_map::get_subscription_count() const
		{
			std::shared_lock<std::shared_timed_mutex> lock(mutex_);

			return entries_delayed_.size() + entries_immediate_.size();
		}

		std::size_t sub_map::get_subscription_count(const topic& topic) const
		{
			std::shared_lock<std::shared_timed_mutex> lock(mutex_);

			std::size_t count = 0;

			count += get_subscription_count(topic, sub::mode::immediate);
			count += get_subscription_count(topic, sub::mode::delayed);

			return count;
		}

		std::size_t sub_map::get_subscription_count(const topic& topic, sub::mode mode) const
		{
			std::shared_lock<std::shared_timed_mutex> lock(mutex_);

			const entry_map* target = &entries_immediate_;

			if (mode == sub::mode::delayed)
			{
				target = &entries_delayed_;
			}

			auto iter = target->find(topic);
			WISE_RETURN_IF(iter == target->end(), 0);

			return iter->second.subs.size();
		}

		sub::key_t sub_map::subscribe(
			entry_map& em,
			const topic& topic,
			sub::cond_t& cond,
			sub::cb_t& cb,
			sub::mode mode)
		{
			auto iter = em.find(topic);

			if (iter == em.end())
			{
				entry e;

				e.topic = topic;

				auto key = subscribe(e, topic, cond, cb, mode);

				em.insert(entry_map::value_type(topic, e));

				return key;
			}
			// else

			return subscribe(iter->second, topic, cond, cb, mode);
		}

		sub::key_t sub_map::subscribe(
			entry& e,
			const topic& topic,
			sub::cond_t& cond,
			sub::cb_t& cb,
			sub::mode mode
		)
		{
			auto key = seq_.next();

			e.subs.push_back(
				sub(key, topic, cond, cb, mode)
			);

			auto ki = keys_.find(key);
			WISE_ASSERT(ki == keys_.end());

			keys_[key] = entry_link{ topic, mode };

			return key;
		}

		bool sub_map::unsubscribe(entry_map& em, sub::key_t key)
		{
			auto iter = keys_.find(key);
			WISE_ASSERT(iter != keys_.end());
			WISE_RETURN_IF(iter == keys_.end(), false);

			auto topic = iter->second.topic;

			keys_.erase(iter);
			seq_.release(key);

			auto ei = em.find(topic);
			WISE_ASSERT(ei != em.end());
			WISE_RETURN_IF(ei == em.end(), false);

			auto& subs = ei->second.subs;

			auto si = std::find_if(
				subs.begin(), subs.end(),
				[key](const sub& s) {return s.get_key() == key; }
			);

			if (si == subs.end())
			{
				return false;
			}

			// 등록할 때 동일 키는 추가되지 않도록 검증

			subs.erase(si);

			// 비면 제거 
			if (subs.empty())
			{
				em.erase(ei);
			}

			return true;
		}

		std::size_t sub_map::post(const topic& topic, message::ptr m, sub::mode mode)
		{
			std::size_t count = 0;

			// posting
			{
				std::shared_lock<std::shared_timed_mutex> lock(mutex_);

				if (mode == sub::mode::immediate)
				{
					count = post(entries_immediate_, topic, m);
				}
				else
				{
					count = post(entries_delayed_, topic, m);
				}
			}

			process_pending_unsubs();

			return count;
		}

		std::size_t sub_map::post(message::ptr m, sub::mode mode)
		{
			std::size_t count = 0;

			// posting
			{
				std::shared_lock<std::shared_timed_mutex> lock(mutex_);

				if (mode == sub::mode::immediate)
				{
					count = post(entries_immediate_, m);
				}
				else
				{
					count = post(entries_delayed_, m);
				}
			}

			process_pending_unsubs();

			return count;
		}

		std::size_t sub_map::post(entry_map& em, const topic& topic, message::ptr m)
		{
			std::size_t count = 0;

			count += post_on_topic(em, topic, m);
			count += post_on_topic(em, topic.get_group_topic(), m);
			count += post_on_topic(em, m->get_topic(), m);
			count += post_on_topic(em, m->get_topic().get_group_topic(), m);

			return count;
		}

		std::size_t sub_map::post(entry_map& em, message::ptr m)
		{
			std::size_t count = 0;

			count += post_on_topic(em, m->get_topic(), m);
			count += post_on_topic(em, m->get_topic().get_group_topic(), m);

			return count;
		}

		std::size_t sub_map::post_on_topic(entry_map& em, const topic& topic, message::ptr m)
		{
			// 테스트 용도 등으로 valid 하지 않은 경우 발생 
			WISE_RETURN_IF(!topic.is_valid(), 0);

			auto iter = em.find(topic);

			if (iter == em.end())
			{
				if (channel_.get_config().log_no_sub_when_post)
				{
					WISE_WARN("{} has no subscription for {}", desc_, m->get_desc());
				}
				return 0;
			}

			auto& subs = iter->second.subs;

			int count = 0;
			auto total = subs.size();

			WISE_ASSERT(total > 0);

			is_posting_ = true;
			posting_thread_ = get_current_thread_hash();

			try
			{
				for (auto& sub : subs)
				{
					if (sub.post(m))
					{
						++count;

						WISE_NONE(
							"posted from channel: {}, msg: {}, posting index: {} of: {}",
							desc_, m->get_desc(), count, total
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

			is_posting_ = false;

			return count;
		}

		void sub_map::process_pending_unsubs()
		{
			// 포스팅 중이면 다음 번에 delayed 포스팅에서 지우도록 한다.
			WISE_RETURN_IF(is_posting_);

			sub::key_t key;

			while (pending_unsubs_.pop(key))
			{
				unsubscribe(key);
			}
		}

		uint64_t sub_map::get_current_thread_hash() const
		{
			// XXX: hash는 논리적으로 중복될 수 있으므로 수정해야 함

			return std::hash<std::thread::id>()(std::this_thread::get_id());
		}

	} // kernel
} // wise
