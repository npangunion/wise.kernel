#include <pch.hpp>
#include <wise.kernel/net/tcp/tcp_addr.hpp>
#include <spdlog/fmt/fmt.h>

namespace wise {
namespace kernel {

tcp_addr::tcp_addr(const std::string& s)
	: raw_(s)
{
	std::size_t pos = s.find_first_of(':');

	WISE_RETURN_IF(pos == std::string::npos);
	WISE_RETURN_IF(pos + 1 >= s.length());

	auto sip = s.substr(0, pos);
	auto sport = s.substr(pos + 1);
	auto port = static_cast<uint16_t>(::atoi(sport.c_str()));

	WISE_RETURN_IF(port == 0);

	boost::system::error_code ec;

	ep_ = tcp::endpoint(boost::asio::ip::address::from_string(sip, ec), port);

	if (!ec)
	{
		valid_ = true;
	}
}

tcp_addr::tcp_addr(const std::string& ip, uint16_t port)
	: tcp_addr(fmt::format("{}:{}", ip, port))
{
}

} // kernel
} // wise
