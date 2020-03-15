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
		const std::string& proto, 
		tcp::socket&& sock, 
		bool accepted) override;
};

} // kernel
} // wise