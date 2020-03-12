#pragma once 

#include <wise.kernel/core/tick.hpp>
#include <wise.kernel/core/mem_tracker.hpp>
#include <wise.kernel/core/message.hpp>
#include <wise.kernel/core/detail/sub_map.hpp>
#include <wise.kernel/core/concurrent_queue.hpp>
#include <atomic>
#include <functional>

namespace wise {
	namespace kernel {

		using skey_t = sub::key_t;

		/// 메세지 전파 채널 
		/**
		 * channel은 메세지를 전달하는 통로를 구성
		 *  topic(uint32_t)과 lambda 함수 (std::function)를 사용
		 *  채널간 연결도 함수를 사용한다.
		 *  토픽은 키 전체, 타잎에 대해서만 subscribe 할 수 있다.
		 *  포스팅할 때 키 전체, 타잎에 대해서만 포스팅 시도한다.
		 *
		 * 생성:
		 *  channel::create(name)
		 *
		 * 사용:
		 *  auto ch = channel::find( "key" );
		 *  auto skey = ch->sub();
		 *  ch->unsub(skey);
		 *
		 * 소유권자:
		 *  채널을 소유하는 곳에서 post() 주기적인 호출
		 *
		 * 소멸:
		 *  channel::destroy(name)
		 *
		 * 등록(sub):
		 *   토픽 단위로 진행
		 *   topic - 받고자 하는 주제 (토픽)
		 *   cond - 조건 체크 (세부 필터링)
		 *   cb - 콜백
		 *
		 * 전파 (post) 모드:
		 *   즉시(immediate) 모드는 publish()할 때 전파.
		 *   지연(delayed) 모드는 post() 호출 시 전파
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
				std::size_t loop_post_limit = 2048;

				/// 단일 메세지 전파 실행 시간이 이 시간을 넘을 경우 로그 남김
				tick_t time_to_log_when_post_time_over = 20;

				/// post() 루프의 전체 전파 실행 시간이 이 시간을 넘을 경우 로그 남김
				tick_t time_to_log_when_post_loop_timeover = 200;

				/// 토픽에 등록된 콜백이 없을 경우 로그 남김
				bool log_no_sub_when_post = false;

				/// 소멸자에서 포스팅 전체 처리하고 종료할 지 여부
				// 안전하지 못 하여 옵션을 제거
				// bool post_all_when_destroyed = true;

				/// 큐에 넣을 필요가 있는 지 먼저 체크. 체크가 더 느릴 가능성이 높음.
				bool check_delayed_sub_before_enqueue = false;

				config() = default;

				config(
					std::size_t lpl,
					tick_t ttl_post_over,
					tick_t ttl_loop_over,
					bool log_no_sub,
					bool check_delayed_sub
				)
					: loop_post_limit(lpl)
					, time_to_log_when_post_time_over(ttl_post_over)
					, time_to_log_when_post_loop_timeover(ttl_loop_over)
					, log_no_sub_when_post(log_no_sub)
					, check_delayed_sub_before_enqueue(check_delayed_sub)
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
			 * @return 생성된 채널. 중복되면 오류로 channel::ptr() 리턴.
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

			/// 생성자 
			/**
			 * key와 구성 정보로 생성
			 *
			 * @param key - 채널의 고유 이름
			 * @param cfg - 구성 정보
			 */
			channel(const key_t& key, const config& cfg);

			/// 소멸자
			~channel();

			/// publish 
			/**
			 * 메세지의 토픽으로도 포스팅.
			 * 큐에 먼저 넣고 포스팅을 진행.
			 * @param m - message
			 *
			 * @return number of dispatched count
			 */
			std::size_t publish(message::ptr m);

			/// publish with a separate topic from message's topic
			/**
			 * 외부에서 제공하는 토픽으로 포스팅.
			 * 이후 메세지의 토픽으로도 포스팅.
			 * 큐에 먼저 넣고 포스팅을 진행.
			 *
			 * @param topic - topic to dispatch first
			 * @param m - message
			 *
			 * dispatch with a topic first. then, call publish(m) inside.
			 *
			 * @return number of dispatched cont
			 */
			std::size_t publish(const topic& topic, message::ptr m);

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
			 * @return 등록된 것 지우면 true, 없으면 false
			 */
			bool unsubscribe(sub::key_t key);

			/// post all messages in queue
			/**
			 * sub::mode::delayed subscriptions are handled for messages
			 */
			std::size_t execute();

			/// post with a message not from a queue. (for net)
			std::size_t publish_immediate(message::ptr m)
			{
				auto count = map_.post(m, sub::mode::immediate);

				stat_.total_immediate_post_count += count;

				return count;
			}

			/// clear all subscriptions
			void clear();

			const config& get_config() const
			{
				return config_;
			}

			/// 키를 돌려줌
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

			std::size_t get_subscription_count(const topic& topic) const
			{
				return map_.get_subscription_count(topic);
			}

			std::size_t get_subscription_count(const topic& topic, sub::mode mode) const
			{
				return map_.get_subscription_count(topic, mode);
			}

		private:
			void enqueue_checked(const topic& topic, message::ptr m);

		private:
			key_t   key_;
			mq_t	q_;
			sub_map map_;
			config  config_;
			stat	stat_;
		};
	} // kernel
} // wise
