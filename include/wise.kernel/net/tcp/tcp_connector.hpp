#pragma once 

#include <wise.kernel/net/reason.hpp>
#include <wise.kernel/core/result.hpp>
#include <wise.kernel/net/tcp/tcp_addr.hpp>
#include <wise.kernel/core/channel/channel.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <memory>

using namespace boost::asio::ip;

namespace wise {
namespace kernel {

class tcp_node;

class tcp_connector final
{
public:
	using result = result<bool, reason>;
	using error_code = boost::system::error_code;

public:
	using ptr = std::shared_ptr<tcp_connector>;

	tcp_connector(
		tcp_node* node,
		uint16_t id,
		const std::string& addr,
		channel::ptr ch);

	~tcp_connector();

	result connect();

	const tcp_addr& get_addr() const
	{
		return addr_;
	}

private:
	void on_connected(const boost::system::error_code& ec);

private:
	tcp_node* node_ = nullptr;
	uint16_t id_ = 0;
	tcp_addr addr_;
	channel::ptr ch_;
	tcp::socket socket_;
};

} // kernel
} // wise
