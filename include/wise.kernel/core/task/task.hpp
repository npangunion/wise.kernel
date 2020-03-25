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

/// 많은 태스크 클래스 중의 하나. 
/**
 * 게임 실행기. 지역, 인던들, 방들 실행.
 *
 * 사용법:
 *   - task의 하위 클래스를 만듦.
 *   - task_scheduler에 등록
 *   - on_start, on_execute, on_finish 재정의 처리
 *   - 필요할 때 자체 finish() 호출
 *   - start() 호출 되지 않으면 런너 쓰레드에서 시작. 실패하면 사라짐
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
	/// 생성자
	task();

	/// 소멸자
	virtual ~task();

	/// 시작. on_start() 호출. 실패 시 false
	bool start();

	/// 특정 runner에서 실행. on_execute() 호출
	result execute(uint32_t runner_id);

	/// 실행을 멈춤
	void pause();

	/// 실행을 재개.
	void resume();

	/// cancel. execute returns exit if cancelled. then finished by task_runner.
	void cancel();

	/// 종료. on_finish() 호출. 
	void finish();

	/// 실시간과 세 가지 레벨
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
	/// 하위 클래스 구현. start()에서 호출
	virtual bool on_start();

	/// 하위 클래스 구현. execute()에서 호출
	virtual result on_execute();

	/// 하위 클래스 구현. finish()에서 호출
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