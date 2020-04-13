#include <pch.hpp>
#include <wise.kernel/server/peer_service.hpp>
#include <wise.kernel/server/server.hpp>
#include <wise.kernel/net/protocol/bits/bits_packets.hpp>

namespace wise {
namespace kernel {

bool peer_service::setup(const nlohmann::json& _json)
{
	WISE_SUBSCRIBE_SELF(bits_syn_accepted, on_accepted);
	WISE_SUBSCRIBE_SELF(bits_syn_connected, on_connected);
	WISE_SUBSCRIBE_SELF(bits_syn_connect_failed, on_connect_failed);
	WISE_SUBSCRIBE_SELF(bits_syn_disconnected, on_disconnected);

	auto jl = _json["listen"];

	if (!jl.is_null())
	{
		auto addr = jl.get<std::string>();
		auto rc = get_server().listen(addr, get_channel());

		if (!rc)
		{
			WISE_ERROR("failed to listen on {}", addr);
			return false;
		}
	}

	auto jc = _json["connect"];

	if (!jc.is_null())
	{
		auto addr = jc.get<std::string>();
		get_server().connect(addr, get_channel());
	}

	return true;
}

bool peer_service::init()
{
	return true;
}

actor::result peer_service::run() 
{
	return result::success;
}

void peer_service::fini()
{

}

void peer_service::on_connected(message::ptr m)
{
	auto ec = cast<bits_syn_connected>(m);
	ec->get_protocol()->bind(get_channel());
}

void peer_service::on_connect_failed(message::ptr m)
{
	auto ec = cast<bits_syn_connect_failed>(m);
	// prepare reconnect
}

void peer_service::on_accepted(message::ptr m)
{
	auto ec = cast<bits_syn_accepted>(m);
	ec->get_protocol()->bind(get_channel());

}
	
void peer_service::on_disconnected(message::ptr m)
{
	// prepare reconnect if the protocol is connected.

}

} // kernel 
} // wise
