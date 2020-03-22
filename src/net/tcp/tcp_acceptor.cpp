#include <pch.hpp>

#include <wise.kernel/net/tcp/tcp_acceptor.hpp>
#include <wise.kernel/net/tcp/tcp_node.hpp>

namespace wise {
namespace kernel {

tcp_acceptor::tcp_acceptor( tcp_node* node, uint16_t id, const std::string& addr)
	: node_(node)
	, id_(id)
	, addr_(addr)
	, socket_(node->ios())
	, acceptor_(node->ios())
{
}

tcp_acceptor::~tcp_acceptor()
{
	acceptor_.close();
}

tcp_acceptor::result tcp_acceptor::listen()
{
	WISE_RETURN_IF(
		!addr_.is_valid(),
		result(false, reason::fail_invalid_address)
	);

	try
	{
		acceptor_.open(addr_.get_endpoint().protocol());
	}
	catch (boost::system::system_error & se)
	{
		WISE_ERROR(
			"tcp_acceptor failed to open. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			se.what()
		);

		return result(true, reason::fail_acceptor_open);
	}

	try
	{
		acceptor_.bind(addr_.get_endpoint());
	}
	catch (boost::system::system_error & se)
	{
		WISE_ERROR(
			"tcp_acceptor failed to bind. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			se.what()
		);

		return result(true, reason::fail_acceptor_bind);
	}

	try
	{
		acceptor_.listen();
	}
	catch (boost::system::system_error & se)
	{
		WISE_ERROR(
			"tcp_acceptor failed to listen. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			se.what()
		);

		return result(true, reason::fail_acceptor_listen);
	}

	WISE_INFO("accepting on addr: {}", addr_.port());

	do_accept();

	return result(true, reason::success);
}

void tcp_acceptor::do_accept()
{
	WISE_RETURN_IF(!acceptor_.is_open());

	acceptor_.async_accept(
		socket_,
		[this](const error_code& ec) {this->on_accepted(ec); }
	);
}

void tcp_acceptor::on_accepted(const error_code& ec)
{
	if (!ec)
	{
		node_->on_accepted(id_, std::move(socket_));
	}
	else
	{
		WISE_ERROR(
			"tcp_acceptor failed to accept. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			ec.message()
		);

		node_->on_accept_failed(id_, ec);
	}

	do_accept();
}

} // kernel
} // wise
