#include "stdafx.h"

#include <wise/net/detail/acceptor.hpp>
#include <wise/net/detail/network_impl.hpp>

namespace wise
{

acceptor::acceptor(uint16_t id, const std::string& protocol, const std::string& addr, uintptr_t pkey)
	: id_(id)
	, addr_(addr)
	, protocol_(protocol)
	, socket_(network::inst().impl().get_ios())
	, acceptor_(network::inst().impl().get_ios())
	, pkey_(pkey)
{
}

acceptor::~acceptor()
{
	acceptor_.close();
}

network::result acceptor::listen()
{
	WISE_RETURN_IF(
		!addr_.is_valid(),
		network::result(false, reason::fail_invalid_address)
	);

	try
	{
		acceptor_.open(addr_.get_endpoint().protocol());
	}
	catch (asio::system_error& se)
	{
		WISE_ERROR(
			"acceptor failed to open. endpoint: {0}, reason: {1}", 
			addr_.get_raw(),  
			se.what()
		);

		return network::result(true, reason::fail_acceptor_open);
	}

	try
	{
		acceptor_.bind(addr_.get_endpoint());
	}
	catch (asio::system_error& se)
	{
		WISE_ERROR(
			"acceptor failed to bind. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			se.what()
		);

		return network::result(true, reason::fail_acceptor_bind);
	}

	try
	{
		acceptor_.listen();
	}
	catch (asio::system_error& se)
	{
		WISE_ERROR(
			"acceptor failed to listen. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			se.what()
		);

		return network::result(true, reason::fail_acceptor_listen);
	}

	WISE_INFO("accepting on addr: {}, protocol: {}", addr_.port(), protocol_);

	do_accept();

	return network::result(true, reason::success);
}

void acceptor::do_accept()
{
	WISE_RETURN_IF(!acceptor_.is_open());

	acceptor_.async_accept(
		socket_,
		[this](const asio::error_code& ec) {this->on_accepted(ec);}
	);
}

void acceptor::on_accepted(const asio::error_code& ec)
{
	if (!ec) 
	{
		network::inst().impl().on_accepted(id_, std::move(socket_));
	}
	else
	{
		WISE_ERROR(
			"acceptor failed to accept. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			ec.message()
		);

		network::inst().impl().on_accept_failed(id_, ec);
	}

	do_accept();
}

} // wise
