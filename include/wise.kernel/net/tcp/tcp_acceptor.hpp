#pragma once 

#include <wise.kernel/net/reason.hpp>
#include <wise.kernel/net/tcp/tcp_addr.hpp>
#include <wise.kernel/core/channel/channel.hpp>
#include <wise.kernel/core/result.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <memory>

using namespace boost::asio::ip;

namespace wise {
namespace kernel {

class tcp_node;

class tcp_acceptor final
{
public:
	using result = result<bool, reason>;
	using error_code = boost::system::error_code;

public:
	using ptr = std::shared_ptr<tcp_acceptor>;

	tcp_acceptor(
		tcp_node* node, 
		uint16_t id, 
		const std::string& proto,
		const std::string& addr, 
		channel::ptr ch);

	~tcp_acceptor();

	/// 지정된 주소에서 accept 시작.
	result listen();

	const tcp_addr& get_addr() const
	{
		return addr_;
	}

	channel::ptr get_channel() const
	{
		return channel_;
	}
	
	const std::string& get_protocol() const
	{
		return proto_;
	}

private:
	void do_accept();

	void on_accepted(const error_code& ec);

private:
	tcp_node* node_ = nullptr;
	uint16_t id_ = 0;
	std::string proto_;
	tcp_addr addr_;
	channel::ptr channel_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

} // kernel
} // wise
