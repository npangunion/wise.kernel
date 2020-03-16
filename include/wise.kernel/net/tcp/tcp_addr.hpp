#pragma once 

#include <wise.kernel/core/macros.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <string>

using namespace boost::asio::ip;

namespace wise {
namespace kernel {

class tcp_addr
{
public:
	tcp_addr(const std::string& s);

	tcp_addr(const std::string& ip, uint16_t port);

	tcp_addr(const tcp_addr& rhs) = default;

	tcp_addr& operator=(const tcp_addr& rhs) = default;

	const std::string& get_raw() const
	{
		return raw_;
	}

	tcp::endpoint get_endpoint() const
	{
		return ep_;
	}

	unsigned short port() const
	{
		return ep_.port();
	}

	std::string ip() const
	{
		return ep_.address().to_string();
	}

	/// 초간단 유효성 체크 결과
	bool is_valid() const
	{
		return valid_;
	}

private:
	std::string	raw_;
	tcp::endpoint ep_;
	bool valid_ = false;
};

} // kernel
} // wise
