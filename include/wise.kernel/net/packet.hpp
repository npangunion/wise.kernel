#pragma once 

#include <wise.kernel/core/channel/message.hpp>

#include <memory>
#include <string>

namespace wise {
namespace kernel {

class session;
using session_ptr = std::shared_ptr<session>;

struct packet : public message
{
	using ptr = std::shared_ptr<packet>;
	using len_t = uint32_t;

	session_ptr session;

	explicit packet(const topic::key_t key)
		: packet(topic(key))
	{
	}

	explicit packet(const topic& topic)
		: message(topic)
	{
	}

	const char* get_desc() const override
	{
		return "packet";
	}
};

} // kernel
} // wise


