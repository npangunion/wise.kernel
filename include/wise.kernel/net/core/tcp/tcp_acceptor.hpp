#pragma once 

#include <wise/net/network.hpp>
#include <wise/net/addr.hpp>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <memory>

using namespace asio::ip;

namespace wise
{

class acceptor final
{
public: 
	using ptr = std::shared_ptr<acceptor>;

	acceptor(uint16_t id, const std::string& protocol, const std::string& addr, uintptr_t pkey);

	~acceptor();

	/// 지정된 주소에서 accept 시작.
	network::result listen(); 

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

	void on_accepted(const asio::error_code& ec);

private: 
	uint16_t id_ = 0;
	std::string protocol_;
	addr addr_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	uintptr_t pkey_ = 0;
};

} // wise
