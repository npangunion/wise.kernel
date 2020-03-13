#pragma once 

#include <wise.kernel/net/packet.hpp>
#include <wise.kernel/net/reason.hpp>
#include <wise.kernel/net/buffer/resize_buffer.hpp>
#include <wise.kernel/core/result.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

namespace wise {
namespace kernel {

/// abstract base class for protocols
class protocol
{
public:
	using ptr = std::shared_ptr<protocol>;
	using result = result<bool, reason>;

public:
	/// constructor
	protocol(const std::string& name) {}

	/// destructor
	virtual ~protocol() {}

    virtual result listen(const std::string& address, int backLog) = 0;

    virtual result connect(const std::string address) = 0;

    virtual void disconnect() = 0;

	/// send to a session after processing message
	virtual result send(packet::ptr m) = 0;

	const std::string& get_name() const
	{
		return name_;
	}

private: 
	std::string name_;
};

} // kernel
} // wise
