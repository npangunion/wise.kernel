#include <pch.hpp>
#include <wise.kernel/net/protocol/bits/bits_node.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>

namespace wise {
namespace kernel {

bits_node::bits_node(const config& _config)
	: tcp_node(_config)
{
}

bits_node::~bits_node()
{
}

protocol::ptr bits_node::create_protocol(
		const std::string& proto,
		tcp::socket&& sock,
		bool accepted) 
{
	auto sp = wise_shared<bits_protocol>(proto, sock, accepted);

	return sp;
}

} // kernel
} // wise
