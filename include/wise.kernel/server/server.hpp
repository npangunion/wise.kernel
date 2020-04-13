#pragma once

#include <wise.kernel/server/actor.hpp>
#include <wise.kernel/server/actor_id_generator.hpp>
#include <wise.kernel/server/actor_directory.hpp>
#include <wise.kernel/server/actor_factory.hpp>
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

	/// create an actor of ACTOR type
	template <typename ACTOR, typename... Args>
	actor::ref create(Args... args);

	/// create an actor of ACTOR type with name
	template <typename ACTOR, typename... Args>
	actor::ref create_with_name(const std::string& name, Args... args);

	/// get actor with id
	actor::ref get_actor(actor::id_t id)
	{
		return actors_.get(id);
	}

	/// get actor with name
	actor::ref get_actor(const std::string& name)
	{
		return actors_.get(name);
	}

private: 
	/// start with default configuratin
	result start();

	/// add actor with id
	actor::ref add_actor(actor::ptr ap);

	/// add actor with id and set name index
	actor::ref add_actor(const std::string& name, actor::ptr ap);

	/// load configuration
	bool load_config();

	/// load server configuration including domain
	bool load_server_config();

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
	actor_directory					actors_;
	actor_id_generator<spinlock>	id_generator_;
	uint16_t						domain_ = 0;
	nlohmann::json					json_;
	std::unique_ptr<bits_node>		bits_node_;
	tcp_node::config				bits_config_;
};

template <typename ACTOR, typename... Args>
actor::ref server::create(Args... args)
{
	auto ap = wise_shared<ACTOR>(
		*this, 
		id_generator_.next(), 
		std::forward<Args>(args)...
	);

	return add_actor(ap);
}

template <typename ACTOR, typename... Args>
actor::ref server::create_with_name(const std::string& name, Args... args)
{
	auto ap = wise_shared<ACTOR>(
		*this,
		id_generator_.next(),
		std::forward<Args>(args)...
		);

	return add_actor(name, ap);
}

} // kernel
} // wise