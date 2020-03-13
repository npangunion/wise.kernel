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
		// �̰ɷ� �ϴ� �� �� �׷���.... ���� �ð����� �ϴ°� ���� ������?
		min_task_count,

		min_task_expected_run_time,
	};

	struct config
	{
		/// runner ����
		int runner_count = 1;

		/// �⺻ �� 1ms. �̺��� ª�� �ɸ��� ����
		float idle_check_threshold_time = 0.001f;

		/// idle�� �� ���� �ð�
		tick_t idle_sleep_time_ms = 1;

		/// 0 : ����. ���� : ���� ����
		unsigned int single_loop_run_limit = 128;

		/// schedule() ȣ�� �� ������ �� �� ����
		bool run_schedule_when_schedule_called = false;

		/// runner���� execute�� ȣ���Ͽ� �����ٸ�
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

	/// ��ü �½�ũ ���� (thread unsafe)
	std::size_t get_task_count() const;

	/// ���ο�. task_runner�� ��� 
	void pass(task::ptr task);

	/// ���ο�. task_runner�����
	bool pop(task::ptr& task);

	/// ���¸� �α׷� ���� 
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