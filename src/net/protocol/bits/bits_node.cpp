#include <pch.hpp>
#include <wise.kernel/net/protocol/bits/bits_node.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>
#include <wise.kernel/net/protocol/bits/bits_packets.hpp>

namespace wise {
namespace kernel {

bits_node::bits_node(const config& _config)
	: tcp_node(_config)
{
}

bits_node::~bits_node()
{
}

protocol::ptr bits_node::create_protocol( tcp::socket&& sock, bool accepted) 
{
	auto sp = wise_shared<bits_protocol>(this, sock, accepted);
	return sp;
}

void bits_node::notify_accepted(tcp_protocol::ptr p)
{
	auto bp = wise_shared<bits_syn_accepted>();
	bp->bind(p);
	p->publish(bp);
}

void bits_node::notify_connected(tcp_protocol::ptr p)
{
	auto bp = wise_shared<bits_syn_connected>();
	bp->bind(p);
	p->publish(bp);
}

void bits_node::notify_connect_failed(
	const std::string& addr,
	const error_code& ec, 
	channel::ptr ch)
{
	auto bp = wise_shared<bits_syn_connect_failed>();
	bp->remote_addr = addr;
	bp->ec = ec;

	ch->publish(bp);
}

void bits_node::notify_disconnect(tcp_protocol::ptr p, const error_code& ec)
{
	auto bp = wise_shared<bits_syn_disconnected>();
	bp->bind(p);
	bp->ec = ec;

	p->publish(bp);
}

} // kernel
} // wise
