#pragma once 

#include <wise.kernel/net/packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>
#include <spdlog/fmt/fmt.h>
#include <bitset>

namespace wise {
namespace kernel {

class bits_packer;

/// base class for zen messages
/**
 */
struct bits_message : public packet
{
	/// type overloading. required for factory
	using ptr = std::shared_ptr<bits_message>;

	/// header length
	static constexpr std::size_t header_length = sizeof(len_t) + sizeof(topic::key_t);

	bits_message(const topic& pic)
		: packet(pic)
	{
		++alloc_;
	}

	bits_message(const topic::key_t key)
		: bits_message(topic(key))
	{
	}

	~bits_message()
	{
		++dealloc_;
	}

	virtual bool pack(bits_packer& packer);

	virtual bool unpack(bits_packer& packer);

	const char* get_desc() const override
	{
		return "bits_message";
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

