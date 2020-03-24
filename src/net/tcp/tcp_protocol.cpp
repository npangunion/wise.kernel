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
	session_ = std::make_unique<tcp_session>(this, std::move(sock));
}

/// destructor
tcp_protocol::~tcp_protocol()
{
}

void tcp_protocol::begin()
{
	session_->begin();
}

void tcp_protocol::disconnect()
{
	session_->disconnect();
}

protocol::result tcp_protocol::send(const uint8_t* bytes, std::size_t len)
{
	return session_->send(bytes, len);
}

const std::string& tcp_protocol::get_local_addr() const
{
	return session_->get_local_addr();
}

const std::string& tcp_protocol::get_remote_addr() const
{
	return session_->get_remote_addr();
}

} // kernel
} // wise