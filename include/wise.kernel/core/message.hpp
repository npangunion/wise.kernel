#pragma once 

#include <wise.kernel/core/topic.hpp>
#include <memory>
#include <string>

namespace wise {
	namespace kernel {

		class message
		{
		public:
			using ptr = std::shared_ptr<message>;
			using proto_t = uint8_t;
			using channel_key_t = std::string;

		public:
			message(const topic& topic = topic(0))
				: topic_(topic)
			{
			}

			virtual ~message()
			{
			}

			const topic& get_topic() const
			{
				return topic_;
			}

			topic& get_topic()
			{
				return topic_;
			}

			// for test 
			void set_topic(const topic& tp)
			{
				topic_ = tp;
			}

			virtual const char* get_desc() const
			{
				return topic::get_desc(topic_).c_str();
			}

		private:
			topic topic_;
		};


		template <typename T>
		inline
			std::shared_ptr<T> cast(message::ptr mp)
		{
			return std::static_pointer_cast<T>(mp);
		}

	} // kernel
} // wise

// macro for channel class callback lambda 
#define WISE_CHANNEL_CB(func) [this](wise::message::ptr mp){ this->func(mp); }

// 매크로로 메세지 타잎을 알 수 있도록 한다.
#define WISE_MESSAGE_DESC(desc) const char* get_desc() const override { return (#desc); }