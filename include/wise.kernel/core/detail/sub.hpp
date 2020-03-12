#pragma once 

#include <wise.kernel/core/message.hpp>
#include <wise.kernel/core/macros.hpp>
#include <functional>

namespace wise {
	namespace kernel {

		class sub
		{
		public:
			using key_t = uint32_t;
			using channel_key_t = message::channel_key_t;
			using cond_t = std::function<bool(message::ptr)>;
			using cb_t = std::function<void(message::ptr)>;

			enum mode
			{
				immediate,  // dispatched when push
				delayed		// dispatched when post
			};

		public:
			sub(key_t key, const topic& topic, cond_t& cond, cb_t cb, sub::mode mode)
				: key_(key)
				, topic_(topic)
				, cond_(cond)
				, cb_(cb)
				, mode_(mode)
			{
			}

			key_t get_key() const
			{
				return key_;
			}

			bool post(message::ptr m)
			{
				WISE_ASSERT(topic_.is_valid());
				WISE_ASSERT(m->get_topic().is_valid());

				WISE_RETURN_IF(!topic_.match(m->get_topic()), false);
				WISE_RETURN_IF(!cond_(m), false);

				cb_(m);

				++post_count_;

				return true;
			}

			const topic& get_topic() const
			{
				return topic_;
			}

			mode get_mode() const
			{
				return mode_;
			}

			std::size_t get_post_count() const
			{
				return post_count_;
			}

		private:
			key_t				key_;
			topic				topic_;
			cond_t				cond_;
			cb_t				cb_;
			mode				mode_;
			std::size_t			post_count_ = 0;
		};

	} // kernel
} // wise
