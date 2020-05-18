#pragma once

#include <wise.kernel/server/actor_directory.hpp>
#include <wise.kernel/server/actor_id_generator.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>
#include <wise.kernel/core/tick.hpp>
#include <wise.kernel/core/timer.hpp>
#include <wise.kernel/util/json.hpp>

namespace wise {
namespace kernel {

class server;

/// actor_cluster manages local and remote actors
class actor_cluster 
{
public:
	actor_cluster(server& _server)
		: server_(_server)
	{
	}

	bool setup(const nlohmann::json& _json);

	bool start();

	void run();

	void finish();

	actor::ref create(const nlohmann::json& _json);

	/// create an actor of ACTOR type
	template <typename ACTOR, typename... Args>
	actor::ref create(Args... args);

	/// create an actor of ACTOR type with name
	template <typename ACTOR, typename... Args>
	actor::ref create_with_name(const std::string& name, Args... args);

	/// get actor with id
	actor::ref get_actor(actor::id_t id)
	{
		return directory_.get(id);
	}

	/// get actor with name
	actor::ref get_actor(const std::string& name)
	{
		return directory_.get(name);
	}
	
private:
	struct remote
	{
		enum class state
		{
			created,
			connecting,
			disconnected,
			connected
		};

		state		state_ = state::created;
		std::string addr_;
		simple_tick tick_; // connected tick
	};

	struct peer
	{
		bits_protocol::ptr protocol_;
		uint16_t		   domain_;
	};

	using remote_map = std::map<std::string, remote>;
	using peer_map = std::map<protocol::id_t, peer>;

private:
	void on_syn_peer_up(message::ptr m);

	void on_syn_peer_down(message::ptr m);

	void on_syn_actor_up(message::ptr m);

	void on_syn_actor_down(message::ptr m);

	void on_peer_down(uint16_t domain);

	void on_connected(message::ptr m);

	void on_connect_failed(message::ptr m);

	void on_accepted(message::ptr m);
	
	void on_disconnected(message::ptr m);

	void reconnect(const std::string& addr);

	void send_syn_peer_up(protocol::ptr pp);

	timer& get_timer()
	{
		return timer_;
	}

private:
	server&							server_;
	tick_t							reconnet_interval_ = 5000;
	remote_map						remotes_;
	peer_map						peers_;
	std::string						peer_addr_;
	std::string						client_addr_;
	actor_directory					directory_;
	actor_id_generator<spinlock>	id_generator_;
	timer							timer_;
};

template <typename ACTOR, typename... Args>
actor::ref actor_cluster::create(Args... args)
{
	auto ap = wise_shared<ACTOR>(
		*this,
		id_generator_.next(),
		std::forward<Args>(args)...
		);

	return directory_.add(ap);
}

template <typename ACTOR, typename... Args>
actor::ref actor_cluster::create_with_name(const std::string& name, Args... args)
{
	auto ap = wise_shared<ACTOR>(
		*this,
		id_generator_.next(),
		std::forward<Args>(args)...
		);

	return directory_.add(name, ap);
}

} // kernel
} // wise