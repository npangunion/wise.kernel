#include "stdafx.h"

#include <wise/net/detail/connector.hpp>
#include <wise/net/detail/network_impl.hpp>

namespace wise
{

connector::connector(uint16_t id, const std::string& protocol, const std::string& addr, uintptr_t pkey)
	: id_(id)
	, protocol_(protocol)
	, addr_(addr)
	, socket_(network::inst().impl().get_ios())
	, pkey_(pkey)
{
}

connector::~connector()
{
}

network::result connector::connect()
{
	// 의도적으로 callback으로 실패하게 만듦

	socket_.async_connect(
		addr_.get_endpoint(), 
		[this](const asio::error_code& ec) { on_connected(ec); }
	);

	return network::result(true, success);
}

void connector::on_connected(const asio::error_code& ec)
{
	if (!ec) // 이 쪽이 성공
	{
		network::inst().impl().on_connected(id_, std::move(socket_));
	}
	else
	{
		WISE_ERROR(
			"connector failed to connect. endpoint: {0}, reason: {1}",
			addr_.get_raw(),
			ec.message()
		);

		network::inst().impl().on_connect_failed(id_, ec);
	}
}

} // wise
