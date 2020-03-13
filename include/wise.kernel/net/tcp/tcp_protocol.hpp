#pragma once

#include <wise.kernel/net/protocol.hpp>

namespace wise {
namespace kernel {

/// tcp protocol
class tcp_protocol : public protocol
{
public:
	/// constructor
	tcp_protocol(const std::string& name) 
		: protocol(name)
	{
	}

	/// destructor
	virtual ~tcp_protocol();

	result listen(const std::string& address, int backLog) override;

	result connect(const std::string address) override;

	void disconnect() override;

	result send(packet::ptr m) override;

	result on_recv(const uint8_t* const bytes, std::size_t len);

	/// session calls this when sent data 
	void on_send(std::size_t len);

	/// session calls this when error ocurrs
	void on_error(const boost::system::error_code& ec);

	const std::string& get_name() const
	{
		return name_;
	}

private:
	std::string name_;
};

} // kernel
} // wise