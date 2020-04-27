#pragma once

#include <wise.kernel/server/actor.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>
#include <wise.kernel/core/tick.hpp>

namespace wise {
namespace kernel {

class probe_actor : public actor
{
public:
	probe_actor(server& _server, id_t id)
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

private:

};

} // kernel
} // wise