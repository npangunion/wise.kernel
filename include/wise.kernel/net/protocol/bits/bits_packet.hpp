#pragma once 

#include <wise.kernel/net/packet.hpp>
#include <wise.kernel/net/protocol.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>
#include <wise.kernel/core/fmt.hpp>
#include <bitset>

namespace wise {
namespace kernel {

class bits_packer;

/// base class for bits messages
/**
 */
struct bits_packet : public packet
{
	/// type overloading. required for factory
	using ptr = std::shared_ptr<bits_packet>;

	/// header length
	static constexpr std::size_t header_length = sizeof(len_t) + sizeof(topic::key_t);

	bits_packet(const topic& pic)
		: packet(pic)
	{
		++alloc_;
	}

	bits_packet(const topic::key_t key)
		: bits_packet(topic(key))
	{
	}

	~bits_packet()
	{
		++dealloc_;
	}

	/// pack(serialize) this packet
	virtual bool pack(bits_packer& packer);

	/// unpack(deserialize) this packet
	virtual bool unpack(bits_packer& packer);

	/// send a bits_packet to the bound protocol
	protocol::result send(bits_packet::ptr packet)
	{
		if (!!get_protocol())
		{
			return get_protocol()->send(packet);
		}

		return protocol::result(false, reason::fail_protocol_not_bound);
	}

	const char* get_desc() const override
	{
		return "bits_packet";
	}

	/// modification options
	bool enable_sequence = false;
	bool enable_checksum = false;
	bool enable_cipher = false;

	static std::atomic<std::size_t> alloc_;
	static std::atomic<std::size_t> dealloc_;
};

} // kernel
} // wise

