#pragma once

#include <wise.kernel/server/actor.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>
#include <wise.kernel/core/tick.hpp>

namespace wise {
namespace kernel {

class peer_service : public actor
{
public:
	peer_service(server& _server, id_t id)
		: actor(_server, id)
	{
	}

	bool setup(const nlohmann::json& _json) override;
	
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
	bool init() override;

	result run() override;

	void fini() override;

	void on_syn_peer_up(message::ptr m);

	void on_syn_peer_down(message::ptr m);

	void on_syn_actor_up(message::ptr m);

	void on_syn_actor_down(message::ptr m);

	void on_connected(message::ptr m);

	void on_connect_failed(message::ptr m);

	void on_accepted(message::ptr m);
	
	void on_disconnected(message::ptr m);

	void reconnect(const std::string& addr);

private:
	tick_t		reconnet_interval_ = 5000;
	remote_map	remotes_;
	peer_map	peers_;
	std::string listen_addr_;
};

} // kernel
} // wise