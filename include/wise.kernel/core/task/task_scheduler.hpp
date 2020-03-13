#pragma once 

#include <wise.kernel/core/task/task.hpp>
#include <wise.kernel/core/task/task_runner.hpp>
#include <wise.kernel/core/concurrent_queue.hpp>
#include <vector>

namespace wise {
namespace kernel {

class task_scheduler
{
public:
	enum policy
	{
		// 이걸로 하는 건 좀 그렇네.... 예상 시간으로 하는게 맞지 않을까?
		min_task_count,

		min_task_expected_run_time,
	};

	struct config
	{
		/// runner 개수
		int runner_count = 1;

		/// 기본 값 1ms. 이보다 짧게 걸리면 쉬기
		float idle_check_threshold_time = 0.001f;

		/// idle일 때 쉬는 시간
		tick_t idle_sleep_time_ms = 1;

		/// 0 : 무한. 숫자 : 개수 제한
		unsigned int single_loop_run_limit = 128;

		/// schedule() 호출 시 스케줄 할 지 여부
		bool run_schedule_when_schedule_called = false;

		/// runner에서 execute를 호출하여 스케줄링
		bool run_schedule_from_runner = true;
	};

public:
	task_scheduler();

	~task_scheduler();

	bool start(const config& cfg);

	void add(task::ptr task);

	void schedule();

	void finish();

	std::size_t get_runner_count() const
	{
		return runners_.size();
	}

	/// 전체 태스크 개수 (thread unsafe)
	std::size_t get_task_count() const;

	/// 내부용. task_runner만 사용 
	void pass(task::ptr task);

	/// 내부용. task_runner만사용
	bool pop(task::ptr& task);

	/// 상태를 로그로 남김 
	void log_stat() const;

private:
	using task_queue = concurrent_queue<task::ptr>;
	using runners = std::vector<task_runner::ptr>;

	void finish_queue(task_queue& q);

private:
	config config_;

	task_queue run_queue_;

	task_queue wait_queue_1_;
	task_queue wait_queue_2_;
	task_queue wait_queue_3_;

	simple_tick st_1_;
	simple_tick st_2_;
	simple_tick st_3_;

	runners runners_;
};

} // kernel
} // wise