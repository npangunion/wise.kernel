#pragma once

#include <wise.kernel/server/actor.hpp>
#include <wise.kernel/server/actor_directory.hpp>
#include <wise.kernel/core/task/task_scheduler.hpp>
#include <wise.kernel/core/result.hpp>
#include <wise.kernel/core/suid_generator.hpp>
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
	std::shared_ptr<ACTOR> create(Args... args);

	actor::ptr get_local_actor(actor::id_t id)
	{
		return local_actors_.get(id);
	}

private: 
	bool load_config();

private: 
	task_scheduler				scheduler_;
	task_scheduler::config		scheduler_cfg_;
	actor_directory				local_actors_;
	suid_generator<spinlock>	actor_id_generator_;
	uint16_t					domain_ = 0;
};

template <typename ACTOR, typename... Args>
std::shared_ptr<ACTOR> server::create(Args... args)
{
	auto ap = wise_shared<ACTOR>(
		*this, 
		actor_id_generator_.next(), 
		std::forward<Args>(args)...
	);

	local_actors_.add(ap);
	scheduler_.add(ap);

	return ap;
}

} // kernel
} // wise