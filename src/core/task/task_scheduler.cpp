#include <pch.hpp>
#include <wise.kernel/core/task_scheduler.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/mem_tracker.hpp>

namespace wise {
	namespace kernel {

		constexpr int level_1 = 10;
		constexpr int level_2 = 20;
		constexpr int level_3 = 100;

		task_scheduler::task_scheduler()
		{

		}

		task_scheduler::~task_scheduler()
		{

		}

		bool task_scheduler::start(const config& cfg)
		{
			WISE_ASSERT(cfg.runner_count > 0);

			config_ = cfg;

			for (auto i = 0; i < cfg.runner_count; ++i)
			{
				task_runner::config config;

				config.idle_check_threshold_time = config_.idle_check_threshold_time;
				config.idle_sleep_time_ms = config_.idle_sleep_time_ms;
				config.single_loop_run_limit = config_.single_loop_run_limit;
				config.run_schedule_from_runner = config_.run_schedule_from_runner;

				auto runner = wise_shared<task_runner>(*this, i + 1);
				auto rc = runner->start(config);

				if (rc)
				{
					runners_.push_back(runner);
				}
			}

			WISE_INFO(
				"task_scheduler started. {} runners", get_runner_count()
			);

			return get_runner_count() > 0;
		}


		void task_scheduler::add(task::ptr task)
		{
			// task can be pushed whether it's ready or not.
			// when it is not ready, it will be started

			run_queue_.push(task);
		}

		void task_scheduler::schedule()
		{
			// called only from one thread.

			task::ptr task;

			if (st_1_.check_timeout(level_1))
			{
				while (wait_queue_1_.pop(task))
				{
					task->mark_queue_time();
					run_queue_.push(task);
				}
			}

			if (st_2_.check_timeout(level_2))
			{
				while (wait_queue_2_.pop(task))
				{
					task->mark_queue_time();
					run_queue_.push(task);
				}
			}

			if (st_3_.check_timeout(level_3))
			{
				while (wait_queue_3_.pop(task))
				{
					task->mark_queue_time();
					run_queue_.push(task);
				}
			}
		}

		void task_scheduler::finish()
		{
			for (auto& r : runners_)
			{
				r->finish();
			}

			WISE_INFO("task_scheduler finishing w/ {} tasks...", run_queue_.unsafe_size());

			finish_queue(run_queue_);
			finish_queue(wait_queue_1_);
			finish_queue(wait_queue_2_);
			finish_queue(wait_queue_3_);

			WISE_INFO("task_scheduler finished");
		}

		void task_scheduler::finish_queue(task_queue& q)
		{
			task::ptr task;

			while (q.pop(task))
			{
				if (task->get_state() == task::state::started ||
					task->get_state() == task::state::cancelled)
				{
					task->finish();
				}
			}

			WISE_ENSURE(q.empty());
		}

		std::size_t task_scheduler::get_task_count() const
		{
			return run_queue_.unsafe_size();
		}

		void task_scheduler::pass(task::ptr task)
		{
			// 3 단계의 스케줄링 간격을 둔다. 
			// 10ms 이하는 실시간으로 본다. 
			// 게임 인스턴스는 대부분 50~100ms면 충분하다. 
			// 실행 중 필요하면 조절할 수 있다. 

			auto lv = task->get_priority();

			switch (lv)
			{
			case task::priority::realtime:
				task->mark_queue_time();
				run_queue_.push(task);
				break;
			case task::priority::level_1:
				wait_queue_1_.push(task);
				break;
			case task::priority::level_2:
				wait_queue_2_.push(task);
				break;
			default:
				wait_queue_3_.push(task);
				break;
			}
		}

		bool task_scheduler::pop(task::ptr& task)
		{
			return run_queue_.pop(task);
		}

		void task_scheduler::log_stat() const
		{
			WISE_INFO(
				"task_scheduler. runners: {}, queue: {}",
				runners_.size(), run_queue_.unsafe_size()
			);

			for (auto& r : runners_)
			{
				r->log_stat();
			}
		}

	} // kernel
} // wise
