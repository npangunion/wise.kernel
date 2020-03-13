#pragma once 

#include <wise.kernel/net/tcp/tcp_addr.hpp>
#include <wise.kernel/core/result.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <memory>

using namespace boost::asio::ip;

namespace wise {
namespace kernel {

class acceptor final
{
public:
	using result = result<bool, std::string>;

public:
	using ptr = std::shared_ptr<acceptor>;

	acceptor(uint16_t id, const std::string& protocol, const std::string& addr, uintptr_t pkey);

	~acceptor();

	/// 지정된 주소에서 accept 시작.
	result listen();

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
	void do_accept();

	void on_accepted(const boost::system::error_code& ec);

private:
	uint16_t id_ = 0;
	std::string protocol_;
	addr addr_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	uintptr_t pkey_ = 0;
};

} // kernel
} // wise
