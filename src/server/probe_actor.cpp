#include <pch.hpp>
#include <wise.kernel/server/probe_actor.hpp>
#include <wise.kernel/server/server.hpp>
#include <wise.kernel/server/server_packets.hpp>
#include <wise.kernel/net/protocol/bits/bits_packets.hpp>

using namespace cluster::messages;

namespace wise {
namespace kernel {

bool probe_actor::setup(const nlohmann::json& _json)
{
	return true;
}

bool probe_actor::init()
{
	// 서버 메세지
	auto aref = server_.get_local_actor("peer_service");

	auto sid = aref.bind(syn_peer_up, get_channel());
	auto sid = aref.bind(syn_peer_down, get_channel());

	WISE_SUBSCRIBE_SELF(syn_peer_up, on_syn_peer_up);
	WISE_SUBSCRIBE_SELF(syn_peer_down, on_syn_peer_down);

	WISE_INFO("probe_actor initialized");

	return true;
}

actor::result probe_actor::run() 
{
	return result::success;
}

void probe_actor::fini()
{

	WISE_INFO("probe_actor finished");
}

void probe_actor::on_syn_peer_up(message::ptr m)
{
	// 해당 연결에 subscribe
}

void probe_actor::on_syn_peer_down(message::ptr m)
{
}

void probe_actor::on_syn_actor_up(message::ptr m)
{
}

void probe_actor::on_syn_actor_down(message::ptr m)
{
}

void probe_actor::on_peer_down(uint16_t domain)
{
	WISE_INFO("peer down. domain:{}", domain);

}

} // kernel 
} // wise
