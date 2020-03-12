#include <pch.hpp>
#include <wise.kernel/core/task_runner.hpp>
#include <wise.kernel/core/task_scheduler.hpp>
#include <wise.kernel/core/tick.hpp>
#include <wise.kernel/core/exception.hpp>
#include <wise.kernel/core/logger.hpp>

namespace wise {
	namespace kernel {

		task_runner::task_runner(task_scheduler& scheduler, uint32_t id)
			: scheduler_(scheduler)
			, id_(id)
		{
		}

		task_runner::~task_runner()
		{
			if (!stop_)
			{
				finish();
			}
		}

		bool task_runner::start(const config& cfg)
		{
			WISE_ASSERT(cfg.idle_check_threshold_time >= 0.f);
			WISE_ASSERT(cfg.single_loop_run_limit >= 0);
			WISE_ASSERT(stop_);

			config_ = cfg;
			stop_ = false;

			WISE_INFO("task_runner starts");

			std::thread([this]() { run(); }).swap(thread_);

			return true;
		}

		void task_runner::finish()
		{
			WISE_ASSERT(!stop_);

			stop_ = true;
			thread_.join();

			WISE_INFO("task_runner finished");
		}

		void task_runner::log_stat() const
		{
			WISE_INFO(
				"id: {}, task_run_count: {}, average_loop_time: {}, total_sleep_count: {}",
				id_, task_run_count_, average_loop_time_, total_sleep_count_
			);
		}

		void task_runner::run()
		{
			simple_tick loop_timer_;

			while (!stop_)
			{
				unsigned int loop_count = 0;
				loop_timer_.reset();

				task::ptr task;

				while (!stop_ && scheduler_.pop(task))
				{
					try
					{
						WISE_ASSERT(task->get_state() != task::state::finished);

						if (task->get_state() == task::state::constructed)
						{
							auto rc = task->start();

							if (!rc)
							{
								WISE_ERROR("Cannot start task:{}, result:{}", task->get_desc(), rc);
								continue;
							}
						}

						if (task->get_state() == task::state::paused)
						{
							scheduler_.pass(task);
							continue;
						}

						auto rc = task->execute(id_);

						switch (rc)
						{
						case task::result::fail:
							WISE_INFO("task {} failed.", task->get_desc());
							task->finish();
							break;

						case task::result::exit:
							WISE_INFO("task {} exit.", task->get_desc());
							task->finish();
							break;
						case task::result::success:
							// stop scheduling
							scheduler_.pass(task);
							break;
						}

						++loop_count;
						++task_run_count_;

						if (config_.single_loop_run_limit > 0 &&
							loop_count >= config_.single_loop_run_limit)
						{
							break;
						}
					}
					catch (std::exception & ex)
					{
						WISE_ERROR("Exception: {} from task: {}", ex.what(), task->get_desc());
						// task->finish();
						// 예외가 발생하면 종료가 안될 가능성이 높다. 
					}
					catch (...)
					{
						WISE_ERROR("Unknown exception from task: {}", task->get_desc());
						// task->finish();
					}
				}

				last_loop_time_ = loop_timer_.elapsed() * 0.001f;
				total_loop_time_ += last_loop_time_;
				average_loop_time_ = (last_loop_time_ + average_loop_time_) / 2;

				if (last_loop_time_ < config_.idle_check_threshold_time)
				{
					std::size_t sleep_time = config_.idle_sleep_time_ms;

					if (loop_count == 0)
					{
						++continous_sleep_count_;

						sleep_time = std::min<std::size_t>(
							100, config_.idle_sleep_time_ms * continous_sleep_count_
							);
					}
					else
					{
						continous_sleep_count_ = 0;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));

					++total_sleep_count_;
				}
			}
		}

	} // kernel
} // wise

