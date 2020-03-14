#pragma once 

#include <wise/net/session.hpp>
#include <wise/net/packet.hpp>
#include <wise/net/protocol/zen/zen_packer.hpp>
#include <spdlog/fmt/fmt.h>
#include <bitset>

namespace wise
{

class zen_packer;

/// base class for zen messages
/**
 */
struct zen_message : public packet
{
	/// type overloading. required for factory
	using ptr = std::shared_ptr<zen_message>;

	/// header length
	static constexpr std::size_t header_length = sizeof(len_t) + sizeof(topic::key_t);

	zen_message(const topic& pic)
		: packet(pic)
	{
		++alloc_;
	}

	zen_message(const topic::key_t key)
		: zen_message(topic(key))
	{
	}

	~zen_message()
	{
		++dealloc_;
	}

	virtual bool pack(zen_packer& packer);

	virtual bool unpack(zen_packer& packer);

	const char* get_desc() const override
	{
		return "zen_message";
	}

	/// modification options
	bool enable_sequence = false;
	bool enable_checksum = false;
	bool enable_cipher = false;

	static std::atomic<std::size_t> alloc_;
	static std::atomic<std::size_t> dealloc_;
};

} // wise

