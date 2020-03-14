#pragma once 

#include <wise.kernel/core/channel/message.hpp>

#include <memory>
#include <string>

namespace wise {
namespace kernel {

class protocol;
using protocol_ptr = std::shared_ptr<protocol>;

class packet : public message
{
public:
	using ptr = std::shared_ptr<packet>;
	using len_t = uint32_t;

public:
	explicit packet(const topic::key_t key)
		: packet(topic(key))
	{
	}

	explicit packet(const topic& topic)
		: message(topic)
	{
	}

	void bind(protocol_ptr proto)
	{
		protocol_ = proto;
	}

	const char* get_desc() const override
	{
		return "packet";
	}

private:
	protocol_ptr protocol_;
};

} // kernel
} // wise


