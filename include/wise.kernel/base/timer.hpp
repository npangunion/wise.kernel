#pragma once 

#include <wise.kernel/base/macros.hpp>
#include <wise.kernel/base/sequence.hpp>
#include <wise.kernel/base/result.hpp>
#include <wise.kernel/base/tick.hpp>
#include <functional>
#include <vector>
#include <queue>
#include <unordered_map>

namespace wise {
	namespace kernel {

		/// timer that checks efficiently
		/**
		 * interval based timer
		 *
		 * Usage:
		 *   auto id = get_timer().set( 10, .... )    // every 10 sec
		 *   get_timer().add( id, on_update_state )   // set callback
		 *
		 *   on_update_state( id )
		 *   {
		 *      send_to_servers( state )
		 *   }
		 *
		 * Performance:
		 * - less than O(nlogn) where n is the reqs in a slot
		 */
		class timer
		{
		public:

			using id_t = int;
			using action = std::function<void(id_t)>;
			using actions = std::vector<action>;

			static constexpr tick_t forever = 0;
			static constexpr tick_t min_interval = 10;

		public:
			/// constructor
			timer();

			~timer();

			/// sets interval
			id_t set(
				tick_t interval,
				tick_t duration = forever,
				bool repeat = true,
				tick_t after = 0.f);

			/// only "once" call is allowed in timer callback
			id_t once(tick_t after, action act);

			/// interval 간격으로 무한 반복 설정
			id_t repeat(tick_t interval, action act);

			/// check slots to run 
			void execute();

			/// add callback action to id req 
			bool add(id_t id, action&& act);

			/// cancel the req (remove)
			bool cancel(id_t id);

			/// VERIFY 
			bool has(id_t id) const;

			/// get last run tick
			tick_t get_last_run_tick(id_t id) const;

			/// get next scheduled tick
			tick_t get_next_run_tick(id_t id) const;

			/// wise purpose
			std::size_t get_slot_count() const
			{
				return slots_.size();
			}

			/// wise purpose
			std::size_t get_req_count() const
			{
				return reqs_.size();
			}

			/// wise purpose
			std::size_t get_run_count(id_t id) const;

		private:
			struct req
			{
				using ptr = std::shared_ptr<req>;

				req() = default;

				req(timer& t,
					id_t id,
					tick_t interval, tick_t duration, tick_t after, bool repeat);

				~req()
				{
					release();
				}

				/// add callback actions
				req& add(action&& act)
				{
					actions_.push_back(std::move(act));
					return *this;
				}

				/// call back 
				void run();

				/// release timer id
				void release();

				/// disable copy && assignment
				req(const req& rhs) = delete;
				req& operator=(req& rhs) = delete;

				// members.

				timer& timer_;
				id_t id_ = 0;
				tick_t interval_ = 0;
				tick_t duration_ = forever;
				tick_t after_ = 0;
				bool repeat_ = true;
				actions actions_;

				tick_t last_run_tick_ = 0;
				tick_t next_run_tick_ = 0;
				tick_t end_run_tick_ = 0;

				bool cancel_ = false;
				std::size_t run_count_ = 0;
			};

			struct slot
			{
				struct entry
				{
					req::ptr ptr_;

					entry(req::ptr p)
						: ptr_(p)
					{
					}

					bool operator < (const entry& rhs) const
					{
						return ptr_->next_run_tick_ > rhs.ptr_->next_run_tick_;
					}
				};

				tick_t interval_ = 0;
				tick_t last_run_tick_ = 0;
				std::priority_queue<entry> reqs_;
			};

			using slots = std::deque<slot>;
			using reqs = std::unordered_map<id_t, req::ptr>;

		private:

			/// 적절한 interval 간격 슬롯이 있는 지 찾고 없으면 생성
			slot& create_slot(tick_t interval);

			/// 슬롯 타이머 처리
			void update_slot(slot& slt, tick_t tick);

			/// remove req
			void remove_end_req(id_t id);

			tick_t get_current_tick() const
			{
				return timer_.elapsed();
			}

		private:
			simple_tick timer_;
			slots slots_;
			reqs reqs_;
			sequence<uint32_t> seq_; // timer id sequence
			bool is_set_during_posting_ = false;
		};


		class timer_holder
		{
		public:
			timer_holder(timer& _timer);

			~timer_holder();

			/// only "once" call is allowed in timer callback
			timer::id_t once(tick_t after, timer::action act);

			/// interval 간격으로 무한 반복 설정
			timer::id_t repeat(tick_t interval, timer::action act);

			/// cancel the timer
			void cancel();

		private:
			void on_once(int timer);

		private:
			timer& timer_;
			timer::id_t id_ = 0;
			bool once_ = false;
			timer::action once_act_;
		};

	} // kernel
} // wise

// macro for timer class callback lambda 
#define WISE_TIMER_CB(func) [this](wise::kernel::timer::id_t id){ this->func(id); }
