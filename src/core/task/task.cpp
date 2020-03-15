#include <pch.hpp>
#include <wise.kernel/core/task/task.hpp>
#include <wise.kernel/core/exception.hpp>
#include <wise.kernel/core/logger.hpp>
// #include <wise.kernel/core/util.hpp>

namespace wise { 
	namespace kernel {

		task::task()
		{
			WISE_ASSERT(state_ == state::constructed);
		}

		task::~task()
		{
		}

		bool task::start()
		{
			if (on_start())
			{
				state_ = state::started;
			}

			WISE_INFO("task {} started", get_desc());

			return state_ == state::started;
		}

		task::result task::execute(uint32_t runner_id)
		{
			WISE_ASSERT(state_ != state::finished);

			// 먼저 체크
			if (state_ == state::cancelled)
			{
				return result::exit;
			}

			if (state_ == state::paused)
			{
				return result::success;
			}

			last_runner_id_ = runner_id;

			last_waiting_time_ = queue_tick_.elapsed() * 0.001f;
			total_waiting_time_ += last_waiting_time_;

			execution_timer_.reset();

			auto rc = on_execute();

			++execution_count_;

			last_execution_time_ = execution_timer_.elapsed() * 0.001f;
			total_execution_time_ += last_execution_time_;
			average_execution_time_ = (average_execution_time_ + last_execution_time_) / 2;

			return rc;
		}

		void task::pause()
		{
			WISE_RETURN_IF(state_ == state::finished);
			state_ = state::paused;
		}

		void task::resume()
		{
			WISE_ASSERT(state_ == state::paused);
			state_ = state::started;
		}

		void task::cancel()
		{
			WISE_RETURN_IF(state_ == state::finished);
			state_ = state::cancelled;
		}

		void task::finish()
		{
			WISE_INFO("task {} finishing...", get_desc());

			on_finish();

			state_ = state::finished;

			WISE_INFO("task {} finished", get_desc());
		}

		bool task::on_start()
		{
			return true;
		}

		task::result task::on_execute()
		{
			return result::success;
		}

		void task::on_finish()
		{
		}

	} // kernel
} // wise