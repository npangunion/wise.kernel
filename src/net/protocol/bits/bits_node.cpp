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
	WISE_UNUSED(proto); // 동일 노드에서 여러 상위 프로토콜을 지원할 경우만 사용

	auto sp = wise_shared<bits_protocol>(this, sock, accepted);

	return sp;
}

} // kernel
} // wise
