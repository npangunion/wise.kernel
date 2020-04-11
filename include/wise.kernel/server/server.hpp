#pragma once

#include <wise.kernel/server/actor.hpp>
#include <wise.kernel/server/actor_id_generator.hpp>
#include <wise.kernel/server/actor_directory.hpp>
#include <wise.kernel/core/task/task_scheduler.hpp>
#include <wise.kernel/core/result.hpp>
#include <wise.kernel/core/spinlock.hpp>

namespace wise
{
namespace kernel
{

class server
{
public: 
	enum class error_code
	{
		  success
		, fail_start_task_scheduler
		, fail_load_config
	};

	using result = result<bool, error_code>;

public: 
	server(); 

	~server();

	result start();

	result run();

	void finish();

	template <typename ACTOR, typename... Args>
	actor::ref create(Args... args);

	actor::ref get_actor(actor::id_t id)
	{
		return actors_.get(id);
	}

private: 
	bool load_config();

private: 
	task_scheduler					scheduler_;
	task_scheduler::config			scheduler_cfg_;
	actor_directory					actors_;
	actor_id_generator<spinlock>	id_generator_;
	uint16_t						domain_ = 0;
};

template <typename ACTOR, typename... Args>
actor::ref server::create(Args... args)
{
	auto ap = wise_shared<ACTOR>(
		*this, 
		id_generator_.next(), 
		std::forward<Args>(args)...
	);

	actors_.add(actor::ref(ap));
	scheduler_.add(ap);

	return actors_.get(ap->get_id());
}

} // kernel
} // wise