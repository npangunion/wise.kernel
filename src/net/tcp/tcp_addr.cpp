#include <pch.hpp>
#include <wise.kernel/net/addr.hpp>
#include <spdlog/fmt/fmt.h>

namespace wise
{

addr::addr(const std::string& s)
	: raw_(s)
{
	std::size_t pos = s.find_first_of(':');

	WISE_RETURN_IF(pos == std::string::npos);
	WISE_RETURN_IF(pos + 1 >= s.length());

	auto sip = s.substr(0, pos);
	auto sport = s.substr(pos+1);
	auto port = static_cast<uint16_t>(::atoi(sport.c_str()));

	WISE_RETURN_IF(port == 0);

	asio::error_code ec;

	ep_ = tcp::endpoint(asio::ip::address::from_string(sip, ec), port);

	if (!ec)
	{
		valid_ = true;
	}
}

addr::addr(const std::string& ip, uint16_t port)
	: addr(fmt::format("{}:{}", ip, port))
{
}

} // wise
