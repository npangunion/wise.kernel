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
	static topic get_topic()
	{
		return topic(
			bits_topic::category, 
			bits_topic::group, 
			(uint16_t)bits_topic::type::connected);
	}

	bits_syn_connected()
		: bits_packet(get_topic())
	{
	}
};

struct bits_syn_connect_failed : public bits_packet
{
	static topic get_topic()
	{
		return topic(
			bits_topic::category,
			bits_topic::group,
			(uint16_t)bits_topic::type::connect_failed);
	}

	std::string remote_addr;
	boost::system::error_code ec;

	bits_syn_connect_failed()
		: bits_packet(get_topic())
	{
	}
};

struct bits_syn_accepted : public bits_packet
{
	static topic get_topic()
	{
		return topic(
			bits_topic::category,
			bits_topic::group,
			(uint16_t)bits_topic::type::accepted);
	}

	bits_syn_accepted()
		: bits_packet(get_topic())
	{
	}
};


struct bits_syn_disconnected: public bits_packet
{
	static topic get_topic()
	{
		return topic(
			bits_topic::category,
			bits_topic::group,
			(uint16_t)bits_topic::type::disconnected);
	}

	boost::system::error_code ec;

	bits_syn_disconnected()
		: bits_packet(get_topic())
	{
	}
};

} // kernel
} // wise