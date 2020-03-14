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
	tcp_protocol();

	/// destructor
	virtual ~tcp_protocol();

	result init(tcp_node* node, tcp::socket&& sock, bool accepted);

	void begin();

	void disconnect();

	bool is_accepted() const
	{
		return accepted_;
	}

	const tcp_node* get_node() const
	{
		return node_;
	}

protected:
	/// tcp_session calls when data received
	virtual result on_recv(const uint8_t* const bytes, std::size_t len) = 0;

	/// tcp_session calls this when sent data 
	virtual void on_send(std::size_t len) = 0;

	/// tcp_session calls this when error ocurrs
	virtual void on_error(const boost::system::error_code& ec) = 0;

private:
	tcp_node* node_ = nullptr;
	tcp_session* session_ = nullptr;
	bool accepted_;
};

} // kernel
} // wise