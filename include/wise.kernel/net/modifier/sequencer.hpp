#pragma once 

#include <wise.kernel/net/modifier/modifier.hpp>
#include <atomic>

namespace wise {
namespace kernel {

/// single byte sequence check
class sequencer final : public modifier
{
public:
	static constexpr std::size_t sequence_size = 1;

	sequencer(protocol* _protocol);

	virtual result begin() override;

	/// after recv. buf has other messages in buffer. 
	virtual result on_recv(
		resize_buffer& buf,
		std::size_t msg_pos,
		std::size_t msg_len,
		std::size_t& new_len
	) override;

	/// before send. buf has only this message
	/**
	 * msg size is increased by 1.
	 * buf is increase by 1.
	 */
	virtual result on_send(
		resize_buffer& buf,
		std::size_t msg_pos,
		std::size_t msg_len
	) override;

private:
	std::atomic<uint8_t> send_seq_ = 0;
	std::atomic<uint8_t> recv_seq_ = 0;
};

} // kernel
} // wise

