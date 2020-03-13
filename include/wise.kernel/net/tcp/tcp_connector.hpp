#pragma once 

#include <wise.kernel/core/result.hpp>
#include <wise.kernel/net/tcp/tcp_addr.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <memory>

using namespace boost::asio::ip;

namespace wise {
namespace kernel {

class connector final
{
public:
	using result = result<bool, std::string>;

public:
	using ptr = std::shared_ptr<connector>;

	connector(uint16_t id, const std::string& protocol, const std::string& addr, uintptr_t pkey);

	~connector();

	result connect();

	const addr& get_addr() const
	{
		return addr_;
	}

	const std::string& get_protocol() const
	{
		return protocol_;
	}

	uintptr_t get_pkey() const
	{
		return pkey_;
	}

private:
	void on_connected(const boost::system::error_code& ec);

private:
	uint16_t id_ = 0;
	std::string protocol_;
	addr addr_;
	tcp::socket socket_;
	uintptr_t pkey_ = 0;
};

} // kernel
} // wise
