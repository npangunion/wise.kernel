#pragma once 

#include <wise/base/macros.hpp>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <string>

using namespace asio::ip;

namespace wise
{

class addr
{
public:
	addr(const std::string& s);

	addr(const std::string& ip, uint16_t port);

	addr(const addr& rhs) = default;

	addr& operator=(const addr& rhs) = default;

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

} // wise
