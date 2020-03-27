#pragma once

#include <wise.kernel/net/protocol.hpp>

using namespace boost::asio::ip;

namespace wise {
namespace kernel {

class tcp_node;
class tcp_session;

/// base tcp protocol
/**
 * 하위 클래스에서 아래 함수 구현
 * - send, on_recv, on_send, on_error 
 */
class tcp_protocol : public protocol
{
	friend class tcp_session;

public:
	using ptr = std::shared_ptr<tcp_protocol>;

public:
	/// constructor
	tcp_protocol(tcp_node* node, tcp::socket&& sock, bool accepted);

	/// destructor
	virtual ~tcp_protocol();

	/// begin communication with tcp_session
	void begin();

	/// close csession from application
	void disconnect();

	/// is accpeted with an acceptor
	bool is_accepted() const
	{
		return accepted_;
	}

	/// ip:port of local host 
	const std::string& get_local_addr() const;

	/// ip:port of remote host
	const std::string& get_remote_addr() const;

	/// get tcp_node
	tcp_node* get_node() 
	{
		return node_;
	}

	/// get const tcp_node
	const tcp_node* get_node() const
	{
		return node_;
	}

	/// app called disconnect
	bool is_active_closed() const
	{
		return active_close_;
	}

protected:
	result send(const uint8_t* bytes, std::size_t len);

protected:
	/// tcp_session calls when data received
	virtual result on_recv(const uint8_t* const bytes, std::size_t len) = 0;

	/// tcp_session calls this when sent data 
	virtual void on_send(std::size_t len) = 0;

	/// tcp_session calls this when error ocurrs
	virtual void on_error(const boost::system::error_code& ec) = 0;

private:
	tcp_node* node_ = nullptr;
	std::unique_ptr<tcp_session> session_;
	bool accepted_ = false;
	bool active_close_ = false;
};

} // kernel
} // wise