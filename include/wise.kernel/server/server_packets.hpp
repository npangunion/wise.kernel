#pragma once
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "server_topics.hpp"

namespace wise {
namespace kernel {

class syn_peer_up : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	uint16_t domain = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(server::cluster::category), 
			static_cast<wise::kernel::topic::group_t>(server::cluster::group), 
			static_cast<wise::kernel::topic::type_t>(server::cluster::syn_peer_up)
		);

		return topic_;
	}

	syn_peer_up()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(domain);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(domain);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_peer_up )
};


class syn_peer_down : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	uint16_t domain = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(server::cluster::category), 
			static_cast<wise::kernel::topic::group_t>(server::cluster::group), 
			static_cast<wise::kernel::topic::type_t>(server::cluster::syn_peer_down)
		);

		return topic_;
	}

	syn_peer_down()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(domain);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(domain);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_peer_down )
};


class syn_actor_up : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	uint64_t id = 0;
	std::string type;
	std::string name;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(server::cluster::category), 
			static_cast<wise::kernel::topic::group_t>(server::cluster::group), 
			static_cast<wise::kernel::topic::type_t>(server::cluster::syn_actor_up)
		);

		return topic_;
	}

	syn_actor_up()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(id);
		packer.pack(type);
		packer.pack(name);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(id);
		packer.unpack(type);
		packer.unpack(name);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_actor_up )
};


class syn_actor_down : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	uint64_t id = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(server::cluster::category), 
			static_cast<wise::kernel::topic::group_t>(server::cluster::group), 
			static_cast<wise::kernel::topic::type_t>(server::cluster::syn_actor_down)
		);

		return topic_;
	}

	syn_actor_down()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(id);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(id);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_actor_down )
};


} // wise
} // kernel


