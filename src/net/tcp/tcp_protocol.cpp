#include <pch.hpp>
#include <wise.kernel/net/tcp/tcp_protocol.hpp>
#include <wise.kernel/net/tcp/tcp_session.hpp>
#include <wise.kernel/net/packet.hpp>

namespace wise {
namespace kernel {

tcp_protocol::tcp_protocol(tcp_node* node, tcp::socket&& sock, bool accepted)
	: protocol()
	, accepted_(accepted)
{
	node_ = node;
	session_ = new tcp_session(this, std::move(sock));
}

/// destructor
tcp_protocol::~tcp_protocol()
{
	delete session_;
}

void tcp_protocol::begin()
{
	session_->begin();
}

void tcp_protocol::disconnect()
{
	session_->disconnect();
}

} // kernel
} // wise