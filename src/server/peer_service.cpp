#include <pch.hpp>
#include <wise.kernel/server/peer_service.hpp>
#include <wise.kernel/server/server.hpp>
#include <wise.kernel/server/server_packets.hpp>
#include <wise.kernel/net/protocol/bits/bits_packets.hpp>

using namespace cluster::messages;

namespace wise {
namespace kernel {

bool peer_service::setup(const nlohmann::json& _json)
{
	// 연결 메세지
	WISE_SUBSCRIBE_SELF(bits_syn_accepted, on_accepted);
	WISE_SUBSCRIBE_SELF(bits_syn_connected, on_connected);
	WISE_SUBSCRIBE_SELF(bits_syn_connect_failed, on_connect_failed);
	WISE_SUBSCRIBE_SELF(bits_syn_disconnected, on_disconnected);

	// 서버 메세지
	WISE_SUBSCRIBE_SELF(syn_peer_up, on_syn_peer_up);
	WISE_SUBSCRIBE_SELF(syn_peer_down, on_syn_peer_down);
	WISE_SUBSCRIBE_SELF(syn_actor_up, on_syn_actor_up);
	WISE_SUBSCRIBE_SELF(syn_actor_down, on_syn_actor_down);

	auto jl = _json.find("listen");

	if (jl != _json.end())
	{
		listen_addr_ = (*jl).get<std::string>();
	}

	auto jri = _json.find("reconnect_interval");

	if (jri != _json.end())
	{
		reconnet_interval_ = (*jri).get<tick_t>();
		WISE_ASSERT(reconnet_interval_ >= 100);
	}

	auto jc = _json.find("connect");

	if (jc != _json.end())
	{
		auto addr = (*jc).get<std::string>();

		remote r; 
		r.state_ = remote::state::created;
		r.addr_ = addr;
		r.tick_.reset();
		remotes_.insert(remote_map::value_type(r.addr_, r));
	}

	return true;
}

bool peer_service::init()
{
	if (listen_addr_.length() > 0)
	{
		auto rc = get_server().listen(listen_addr_, get_channel());

		if (!rc)
		{
			WISE_ERROR("peer_service failed to listen on {}", listen_addr_);
			return false;
		}
	}

	for (auto& rkv : remotes_)
	{
		if (rkv.second.state_ == remote::state::created)
		{
			rkv.second.state_ = remote::state::connecting;
			WISE_INFO("connecting remote {}", rkv.second.addr_);

			get_server().connect(rkv.second.addr_, get_channel());
		}
	}

	WISE_INFO("peer_service initialized");

	return true;
}

actor::result peer_service::run() 
{
	return result::success;
}

void peer_service::fini()
{

	WISE_INFO("peer_service finished");
}

void peer_service::on_syn_peer_up(message::ptr m)
{
	auto ec = cast<syn_peer_up>(m);

}

void peer_service::on_syn_peer_down(message::ptr m)
{
	auto ec = cast<syn_peer_down>(m);

}

void peer_service::on_syn_actor_up(message::ptr m)
{
	auto ec = cast<syn_actor_up>(m);

}

void peer_service::on_syn_actor_down(message::ptr m)
{
	auto ec = cast<syn_actor_down>(m);

}

void peer_service::on_connected(message::ptr m)
{
	auto ec = cast<bits_syn_connected>(m);
	auto pr = ec->get_protocol();
	auto tp = std::static_pointer_cast<tcp_protocol>(pr);
	auto iter = remotes_.find(tp->get_remote_addr());
	if (iter != remotes_.end())
	{
		ec->get_protocol()->bind(get_channel());
		iter->second.state_ = remote::state::connected;
		iter->second.tick_.reset();

		WISE_INFO("peer_service connected to {}", tp->get_remote_addr());
	}
	else
	{
		WISE_ERROR("on_connected. unknown peer. {}", tp->get_remote_addr());
	}
}

void peer_service::on_connect_failed(message::ptr m)
{
	auto ec = cast<bits_syn_connect_failed>(m);

	auto iter = remotes_.find(ec->remote_addr);
	if (iter != remotes_.end())
	{
		ec->get_protocol()->bind(get_channel());
		iter->second.state_ = remote::state::disconnected;

		get_timer().once(reconnet_interval_, 
			[ec, this](timer::id_t ) { reconnect(ec->remote_addr); });
	}
	else
	{
		WISE_ERROR("on_connect_faild. unknown peer. {}", ec->remote_addr);
	}
}

void peer_service::on_accepted(message::ptr m)
{
	auto ec = cast<bits_syn_accepted>(m);
	ec->get_protocol()->bind(get_channel());

	auto tp = std::static_pointer_cast<tcp_protocol>(ec->get_protocol());
	WISE_INFO("peer_service accepted {}", tp->get_remote_addr());
}
	
void peer_service::on_disconnected(message::ptr m)
{
	auto ec = cast<bits_syn_disconnected>(m);

	auto pr = ec->get_protocol();
	auto tp = std::static_pointer_cast<tcp_protocol>(pr);
	auto iter = remotes_.find(tp->get_remote_addr());
	if (iter != remotes_.end())
	{
		iter->second.state_ = remote::state::disconnected;
		get_timer().once(reconnet_interval_, 
			[tp, this] (timer::id_t ) { reconnect(tp->get_remote_addr()); });
	}

	// TODO: process syn_peer_down, syn_actor_down for the remote host
}

void peer_service::reconnect(const std::string& addr)
{
	auto iter = remotes_.find(addr);
	if (iter != remotes_.end())
	{
		iter->second.state_ = remote::state::connecting;
		WISE_INFO("reconnecting {}", addr);

		get_server().connect(addr, get_channel());
	}
}

} // kernel 
} // wise
