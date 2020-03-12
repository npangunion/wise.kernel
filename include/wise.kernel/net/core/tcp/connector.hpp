#pragma once 

#include <wise/net/network.hpp>
#include <wise/net/addr.hpp>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <memory>

using namespace asio::ip;

namespace wise
{

class connector final
{
public:
	using ptr = std::shared_ptr<connector>;

	connector(uint16_t id, const std::string& protocol, const std::string& addr, uintptr_t pkey);

	~connector();

	network::result connect();

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
	void on_connected(const asio::error_code& ec);

private: 
	uint16_t id_ = 0;
	std::string protocol_;
	addr addr_;
	tcp::socket socket_;
	uintptr_t pkey_ = 0;
};

} // wise
