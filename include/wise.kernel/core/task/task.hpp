#pragma once 

#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/core/tick.hpp>
#include <wise.kernel/core/mem_tracker.hpp>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>

namespace wise {
namespace kernel {

/// ���� �½�ũ Ŭ���� ���� �ϳ�. 
/**
 * ���� �����. ����, �δ���, ��� ����.
 *
 * ����:
 *   - task�� ���� Ŭ������ ����.
 *   - task_scheduler�� ���
 *   - on_start, on_execute, on_finish ������ ó��
 *   - �ʿ��� �� ��ü finish() ȣ��
 *   - start() ȣ�� ���� ������ ���� �����忡�� ����. �����ϸ� �����
 */
class task : public std::enable_shared_from_this<task>
{
public:
	using ptr = std::shared_ptr<task>;

	enum class state
	{
		constructed,
		started,
		cancelled,
		paused,
		finished
	};

	enum class result
	{
		success,
		fail,
		exit,
		finished
	};

	enum class priority
	{
		realtime,
		level_1,	// 10 
		level_2,	// 50
		level_3		// 200 ~ ms
	};

public:
	/// ������
	task();

	/// �Ҹ���
	virtual ~task();

	/// ����. on_start() ȣ��. ���� �� false
	bool start();

	/// Ư�� runner���� ����. on_execute() ȣ��
	result execute(uint32_t runner_id);

	/// ������ ����
	void pause();

	/// ������ �簳.
	void resume();

	/// cancel. execute returns exit if cancelled. then finished by task_runner.
	void cancel();

	/// ����. on_finish() ȣ��. 
	void finish();

	/// �ǽð��� �� ���� ����
	void set_priority(priority prio)
	{
		priority_ = prio;
	}

	state get_state() const
	{
		return state_;
	}

	bool is_finished() const
	{
		return state_ == state::finished;
	}

	priority get_priority() const
	{
		return priority_;
	}

	uint32_t get_last_runner_id() const
	{
		return last_runner_id_;
	}

	tick_t get_last_execution_time() const
	{
		return last_execution_time_;
	}

	unsigned int get_execution_count() const
	{
		return execution_count_;
	}

	tick_t get_total_execution_time() const
	{
		return total_execution_time_;
	}

	const std::string& get_desc() const
	{
		return desc_;
	}

	void mark_queue_time()
	{
		queue_tick_.reset();
	}

	tick_t get_total_waiting_time() const
	{
		return total_waiting_time_;
	}

	task(task& rhs) = delete;
	task& operator=(const task& rhs) = delete;

protected:
	void set_desc(const std::string& desc)
	{
		desc_ = desc;
	}

protected:
	/// ���� Ŭ���� ����. start()���� ȣ��
	virtual bool on_start();

	/// ���� Ŭ���� ����. execute()���� ȣ��
	virtual result on_execute();

	/// ���� Ŭ���� ����. finish()���� ȣ��
	virtual void on_finish();

private:
	std::atomic<state> state_ = state::constructed;
	uint32_t last_runner_id_ = 0;
	std::string desc_ = "task unkown";

	simple_tick execution_timer_;
	unsigned int execution_count_ = 0;
	std::atomic<priority> priority_ = priority::level_1;

	tick_t last_execution_time_ = 0;
	tick_t average_execution_time_ = 0;	// moving average
	tick_t total_execution_time_ = 0;
	tick_t last_waiting_time_ = 0;
	tick_t total_waiting_time_ = 0;

	simple_tick queue_tick_;
};

} // kernel
} // wise