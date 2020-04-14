#pragma once

#include <wise.kernel/net/tcp/tcp_node.hpp>

namespace wise {
namespace kernel {

class bits_node : public tcp_node
{
public: 
	bits_node(const config& _config);

	~bits_node();

private: 
	protocol::ptr create_protocol(
		tcp::socket&& sock, 
		bool accepted) override;

	void notify_accepted( tcp_protocol::ptr p) override;

	void notify_connected( tcp_protocol::ptr p) override;

	void notify_connect_failed(
		const std::string& addr, 
		const error_code& ec, 
		channel::ptr ch) override;

	void notify_disconnect(
		tcp_protocol::ptr p, 
		const error_code& ec) override;
};

} // kernel
} // wise