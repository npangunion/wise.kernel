#pragma once 

#include <wise.kernel/core/task/task.hpp>
#include <wise.kernel/core/concurrent_queue.hpp>
#include <atomic>
#include <thread>

namespace wise {
namespace kernel {

class task_scheduler;

class task_runner
{
public:
	struct config
	{
		/// �⺻ �� 1ms. �̺��� ª�� �ɸ��� 1ms ����
		float idle_check_threshold_time = 0.001f;

		/// idle�� �� ���� �ð�
		tick_t idle_sleep_time_ms = 1;

		/// 0 : ����. ���� : ���� ����
		unsigned int single_loop_run_limit = 64;

		/// runner���� execute�� ȣ���Ͽ� �����ٸ�
		bool run_schedule_from_runner = true;
	};

	using ptr = std::shared_ptr<task_runner>;

public:
	task_runner(task_scheduler& scheduler, uint32_t id);

	~task_runner();

	bool start(const config& cfg);

	void finish();

	void log_stat() const;

private:
	void run();

private:
	task_scheduler& scheduler_;
	uint32_t id_ = 0;
	config config_;
	std::atomic<bool> stop_ = true;
	std::thread thread_;

	std::atomic<std::size_t> task_run_count_ = 0;
	std::size_t total_sleep_count_ = 0;
	std::size_t continous_sleep_count_ = 0;
	float last_loop_time_ = 0.f;
	float average_loop_time_ = 0.f;	// moving average
	float total_loop_time_ = 0.f;
};

} // kernel
} // wise