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
 * - and more...
 *
 * TODO: interface�� ���������� context ������ �߰��Ѵ�.
 * - context�� ���ؼ��� ���� �Ǵ�
 * - context�� ������ �����Ѵ�.
 * - ������ ��Ȯ���ϴ�.
 */
class modifier
{
public:
	using result = result<bool, reason>;

public:
	modifier(protocol* _protocol)
		: protocol_(_protocol)
	{}

	virtual ~modifier() = default;

	virtual result begin() = 0;

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
		std::size_t msg_pos,
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
		std::size_t msg_pos,
		std::size_t msg_len
	) = 0;

protected:
	protocol* get_protocol()
	{
		return protocol_;
	}

	/// update length field to reflect change
	void update_length_field(
		resize_buffer& buf,
		std::size_t msg_pos,
		std::size_t new_len
	);

protected:
	std::atomic<bool> bound_ = false;

private:
	protocol* protocol_ = nullptr;
};

} // kernel
} // wise
