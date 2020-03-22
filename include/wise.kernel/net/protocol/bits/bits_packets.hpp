#pragma once

#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <boost/system/error_code.hpp>

namespace wise {
namespace kernel {

struct bits_topic
{
	static const uint8_t category = 1;
	static const uint8_t group = 1;

	enum class type : uint16_t
	{
		connected = 1,
		connect_failed,
		accepted,
		disconnected,
	};

	static topic connected()
	{
		return topic(category, group, (uint16_t)type::connected);
	}

	static topic connect_failed()
	{
		return topic(category, group, (uint16_t)type::connect_failed);
	}

	static topic accepted()
	{
		return topic(category, group, (uint16_t)type::accepted);
	}

	static topic disconnected()
	{
		return topic(category, group, (uint16_t)type::disconnected);
	}
};

// naming convention: 
// - syn : notification wo/ response
// - req : request expecting a response
// - res : response for a req packet

struct bits_syn_connected : public bits_packet
{
	std::string local_addr;
	std::string remote_addr;

	bits_syn_connected()
		: bits_packet(bits_topic::connected())
	{
	}
};

struct bits_syn_connect_failed : public bits_packet
{
	std::string local_addr;
	std::string remote_addr;
	boost::system::error_code ec;
	std::string desc;

	bits_syn_connect_failed()
		: bits_packet(bits_topic::connect_failed())
	{
	}
};

struct bits_syn_accepted : public bits_packet
{
	std::string local_addr;
	std::string remote_addr;

	bits_syn_accepted()
		: bits_packet(bits_topic::accepted())
	{
	}
};


struct bits_syn_disconnected: public bits_packet
{
	std::string local_addr;
	std::string remote_addr;
	boost::system::error_code ec;
	std::string desc;

	bits_syn_disconnected()
		: bits_packet(bits_topic::disconnected())
	{
	}
};

} // kernel
} // wise