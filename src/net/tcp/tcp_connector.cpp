#include <pch.hpp>

#include <wise.kernel/net/tcp/tcp_connector.hpp>
#include <wise.kernel/net/tcp/tcp_node.hpp>

namespace wise {
namespace kernel {

tcp_connector::tcp_connector(
	tcp_node* node,
	uint16_t id,
	const std::string& addr)
	: node_(node)
	, id_(id)
	, addr_(addr)
	, socket_(node->ios())
{
}

tcp_connector::~tcp_connector()
{
}

tcp_connector::result tcp_connector::connect()
{
	// 의도적으로 callback으로 실패하게 만듦

	socket_.async_connect(
		addr_.get_endpoint(),
		[this](const error_code& ec) { on_connected(ec); }
	);

	return result(true, reason::success);
}

void tcp_connector::on_connected(const error_code& ec)
{
	if (!ec) // 이 쪽이 성공
	{
		node_->on_connected(id_, std::move(socket_));
	}
	else
	{
		WISE_ERROR(
			"tcp_connector failed to connect. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			ec.message()
		);

		node_->on_connect_failed(id_, ec);
	}
}

} // kernel
} // wise

