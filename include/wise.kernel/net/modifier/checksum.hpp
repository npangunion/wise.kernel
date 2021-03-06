#pragma once 

#include <wise.kernel/net/modifier.hpp>

namespace wise {
namespace kernel {

class checksum final : public modifier
{
public:
	static constexpr std::size_t checksum_size = 4;

	checksum(std::size_t header_length);

	/// after recv. buf has other messages in buffer. 
	virtual result on_recv(
		resize_buffer& buf,
		std::size_t msg_offset,
		std::size_t msg_len,
		std::size_t& new_len
	) override;

	/// before send. buf has only this message
	virtual result on_send(
		resize_buffer& buf,
		std::size_t msg_offset,
		std::size_t msg_len
	) override;

private:
	std::size_t header_length_;

};

} // kernel
} // wise
