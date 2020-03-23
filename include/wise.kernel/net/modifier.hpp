#pragma once 

#include <wise.kernel/net/buffer/resize_buffer.hpp>
#include <wise.kernel/net/reason.hpp>
#include <wise.kernel/core/result.hpp>

namespace wise {
namespace kernel {

class protocol;

/// modifies and verifies messages
/**
 * modifier is used with each protocol to provide:
 * - sequence check
 * - checksum check
 * - cipher check
 * 
 * NOTE: 4바이트 길이를 갖는 프로토콜만 동작.
 */
class modifier
{
public:
	using result = result<bool, reason>;

public:
	modifier() {}

	virtual ~modifier() = default;

	/// after recv. buf has other messages in buffer. 
	/**
	 * usually it is not necessary to modify buffer.
	 * - just read and verify spec
	 * - thread-safe to read and process buffer
	 *
	 * @param new_len - new message length after processing
	 */
	virtual result on_recv(
		resize_buffer& buf,
		std::size_t msg_offset, // 0으로 대부분 시작
		std::size_t msg_len,
		std::size_t& new_len
	) = 0;

	/// before send. buf has only this message
	/**
	 * must provide thread-safey. called from multiple threads
	 *
	 * changes buffer
	 * - length field of message
	 * - resize (padding, sequence, checksum field)
	 * - each field with encryption
	 */
	virtual result on_send(
		resize_buffer& buf,
		std::size_t msg_offset,
		std::size_t msg_len
	) = 0;

protected:
	/// update length field to reflect change
	void update_length_field(
		resize_buffer& buf,
		std::size_t msg_offset,
		std::size_t new_len
	);

protected:
	std::atomic<bool> bound_ = false;
};

} // kernel
} // wise
