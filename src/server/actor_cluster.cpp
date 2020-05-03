#include <pch.hpp>
#include <wise.kernel/server/actor_cluster.hpp>
#include <wise.kernel/server/server.hpp>
#include <wise.kernel/server/server_packets.hpp>
#include <wise.kernel/net/protocol/bits/bits_packets.hpp>

using namespace cluster::messages;

namespace wise {
namespace kernel {

bool actor_cluster::setup(const nlohmann::json& _json)
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

bool actor_cluster::start()
{
	if (listen_addr_.length() > 0)
	{
		auto rc = get_server().listen(listen_addr_, get_channel());

		if (!rc)
		{
			WISE_ERROR("actor_cluster failed to listen on {}", listen_addr_);
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

	WISE_INFO("actor_cluster initialized");

	return true;
}

void actor_cluster::run() 
{
}

void actor_cluster::finish()
{
	for (auto& kvp : peers_)
	{
		kvp.second.protocol_->disconnect();
	}

	peers_.clear();
	remotes_.clear();

	WISE_INFO("actor_cluster finished");
}

actor::ref actor_cluster::add_actor(actor::ptr ap)
{
	directory_.add(actor::ref(ap));
	get_server().schedule(ap);

	return actors_.get(ap->get_id());
}

actor::ref actor_cluster::add_actor(const std::string& name, actor::ptr ap)
{
	directory_.add(name, actor::ref(ap));
	get_server().schedule(ap);

	return actors_.get(ap->get_id());
}

void actor_cluster::on_syn_peer_up(message::ptr m)
{
	auto ec = cast<syn_peer_up>(m);
	auto iter = peers_.find(ec->get_protocol()->get_id());

	if (iter != peers_.end())
	{
		WISE_ASSERT(iter->second.protocol_ == ec->get_protocol());

		iter->second.domain_ = ec->domain;

		WISE_INFO("peer up. domain:{}, addr:{}", 
			ec->domain, iter->second.protocol_->get_remote_addr());

		if (!iter->second.protocol_->is_accepted())
		{
			// connected host replies to the syn_peer_up
			send_syn_peer_up(iter->second.protocol_);
		}

		// TODO: public 액터들 현재 정보 syn_actor_up으로 전달
	}
}

void actor_cluster::on_syn_peer_down(message::ptr m)
{
	auto ec = cast<syn_peer_down>(m);
	on_peer_down(ec->domain);
}

void actor_cluster::on_syn_actor_up(message::ptr m)
{
	auto ec = cast<syn_actor_up>(m);

}

void actor_cluster::on_syn_actor_down(message::ptr m)
{
	auto ec = cast<syn_actor_down>(m);

}

void actor_cluster::on_peer_down(uint16_t domain)
{
	WISE_INFO("peer down. domain:{}", domain);

	// process actor down on the host of domain
}

void actor_cluster::on_connected(message::ptr m)
{
	auto ec = cast<bits_syn_connected>(m);
	auto pr = ec->get_protocol();
	auto bp = std::static_pointer_cast<bits_protocol>(pr);

	auto iter = remotes_.find(bp->get_remote_addr());

	if (iter != remotes_.end())
	{
		ec->get_protocol()->bind(get_channel());
		iter->second.state_ = remote::state::connected;
		iter->second.tick_.reset();

		peer p;
		p.protocol_ = bp;
		p.domain_ = 0;

		peers_.insert(peer_map::value_type(p.protocol_->get_id(), p));
		WISE_INFO("actor_cluster connected to {}. waiting peer up.", bp->get_remote_addr());
	}
	else
	{
		WISE_ERROR("on_connected. unknown peer. {}", bp->get_remote_addr());
	}
}

void actor_cluster::on_connect_failed(message::ptr m)
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

void actor_cluster::on_accepted(message::ptr m)
{
	auto ec = cast<bits_syn_accepted>(m);
	ec->get_protocol()->bind(get_channel());

	auto bp = std::static_pointer_cast<bits_protocol>(ec->get_protocol());

	peer p; 
	p.protocol_ = bp; 
	p.domain_ = 0;

	peers_.insert(peer_map::value_type(p.protocol_->get_id(), p));
	WISE_INFO("actor_cluster accepted {}", bp->get_remote_addr());

	send_syn_peer_up(p.protocol_);
}
	
void actor_cluster::on_disconnected(message::ptr m)
{
	auto ec = cast<bits_syn_disconnected>(m);

	auto pr = ec->get_protocol();
	auto tp = std::static_pointer_cast<tcp_protocol>(pr);

	// reconnect
	{
		auto iter = remotes_.find(tp->get_remote_addr());

		if (iter != remotes_.end())
		{
			iter->second.state_ = remote::state::disconnected;
			get_timer().once(reconnet_interval_,
				[tp, this](timer::id_t) { reconnect(tp->get_remote_addr()); });
		}
	}

	// peer down
	{
		auto iter = peers_.find(tp->get_id());

		if (iter != peers_.end())
		{
			on_peer_down(iter->second.domain_);
			peers_.erase(iter);

			WISE_INFO("peer disconnected. protocol:0x{:x}, addr:{}", tp->get_id(), tp->get_remote_addr());
		}
		else
		{
			WISE_INFO("unkown peer disconnected. addr:{}", tp->get_remote_addr());
		}
	}
}

void actor_cluster::reconnect(const std::string& addr)
{
	auto iter = remotes_.find(addr);

	if (iter != remotes_.end())
	{
		iter->second.state_ = remote::state::connecting;
		WISE_INFO("reconnecting {}", addr);

		get_server().connect(addr, get_channel());
	}
}

void actor_cluster::send_syn_peer_up(protocol::ptr pp)
{
	auto spu = std::make_shared<syn_peer_up>();
	spu->domain = get_server().get_domain();

	pp->send(spu);

	WISE_DEBUG("syn_peer_up sent to protoocl:0x{:x}", pp->get_id());
}

} // kernel 
} // wise
