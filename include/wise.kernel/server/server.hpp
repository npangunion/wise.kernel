#pragma once

#include <wise.kernel/server/actor.hpp>
#include <wise.kernel/server/actor_id_generator.hpp>
#include <wise.kernel/server/actor_factory.hpp>
#include <wise.kernel/server/actor_cluster.hpp>
#include <wise.kernel/net/protocol/bits/bits_node.hpp>
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
		, fail_start_bits_node
		, fail_start_actor_cluster
		, fail_load_config
		, fail_listen
		, fail_connect
	};

	using result = result<bool, error_code>;

public: 
	server(); 

	~server();

	/// start with configuration 
	result start(const std::string& file);

	/// run by call
	result run();

	/// finish server
	void finish();

	/// listen on address 
	result listen(const std::string& addr, channel::ptr ch);

	/// connec to address
	result connect(const std::string& addr, channel::ptr ch);
	
	/// schedule actor as a task
	void schedule(actor::ptr ap)
	{
		scheduler_.add(ap);
	}

	/// create an actor of ACTOR type
	template <typename ACTOR, typename... Args>
	actor::ref create(Args... args);

	/// create an actor of ACTOR type with name
	template <typename ACTOR, typename... Args>
	actor::ref create_with_name(const std::string& name, Args... args);

	/// get actor with id
	actor::ref get_actor(actor::id_t id)
	{
		return cluster_.get(id);
	}

	/// get actor with name
	actor::ref get_actor(const std::string& name)
	{
		return cluster_.get(name);
	}

	uint16_t get_domain() const
	{
		return domain_;
	}

private: 
	/// start with default configuratin
	result start();

	/// load configuration
	bool load_config();

	/// load server configuration including domain
	bool load_server_config();

	/// load cluster configuration
	bool load_cluster_config();

	/// load bits_node configuration
	bool load_bits_config();

	/// load scheduler configuration
	bool load_scheduler_config();

	/// load actors and create them
	bool load_actors();

private: 
	std::string						config_file_;
	task_scheduler					scheduler_;
	task_scheduler::config			scheduler_cfg_;
	uint16_t						domain_ = 0;
	nlohmann::json					json_;
	std::unique_ptr<bits_node>		bits_node_;
	tcp_node::config				bits_config_;
	actor_cluster					cluster_;
};

template <typename ACTOR, typename... Args>
actor::ref server::create(Args... args)
{
	return cluster_.create<ACTOR>(std::forward(Args)...);
}

template <typename ACTOR, typename... Args>
actor::ref server::create_with_name(const std::string& name, Args... args)
{
	return cluster_.create_with_name<ACTOR>(std::forward(Args)...);
}

} // kernel
} // wise