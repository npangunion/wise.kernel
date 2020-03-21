#pragma once 

#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/tcp/tcp_protocol.hpp>
#include <wise.kernel/net/modifier/sequencer.hpp>
#include <wise.kernel/net/modifier/checksum.hpp>
#include <wise.kernel/net/modifier/cipher.hpp>
#include <wise.kernel/net/buffer/resize_buffer.hpp>

namespace wise {
namespace kernel {

/// a simple message protocol
/**
 * Header
 * - length: 4 bytes
 * - topic : 4 bytes
 *
 * Check:
 * - crc32 : 4 bytes
 * - sequence : 1 byte
 * - cipher (AES128 / CBC)
 *   - key and IV change on recv and send w/ sha1 and obfusfication
 *   - initial random cipher message sent
 */
class bits_protocol final : public tcp_protocol
{
public:
	/// static configuration for bits protocol
	struct config
	{
		bool enable_cipher = true;

		bool enable_checksum = true;

		bool enable_sequence = true;

		bool enable_detail_log = false;

		bool enable_loopback = false;

		std::size_t max_packet_length = 512 * 1024;  // 512K
	};

	static config cfg;

public:
	/// constructor
	bits_protocol(tcp_node* node, tcp::socket& sock, bool accepted);

	/// ensure session is closed
	~bits_protocol();

	/// override protocol::send 
	result send(packet::ptr m) override;

	/// 수신 처리 테스트를 위한 함수. 부분 수신 등 확인 용도
	result on_recv_to_test(
		const uint8_t* const bytes,
		std::size_t len
	);

	/// internal. made public to test
	result bits_protocol::pack(bits_packet::ptr mp, resize_buffer& buf);

private:

	/// cipher, checksum, then send to session
	result send_final(
		bits_packet::ptr mp,
		const uint8_t* const data,
		std::size_t len
	);

	result send_final(
		bits_packet::ptr mp,
		resize_buffer& buf,
		std::size_t len
	);

	result send_modified(
		bits_packet::ptr mp,
		resize_buffer& buf,
		std::size_t len
	);

	result recv_modified(
		bits_packet::ptr mp,
		resize_buffer& buf,
		std::size_t msg_pos,
		std::size_t msg_len,
		std::size_t& final_len
	);

	/// tcp_protocol::on_recv
	result on_recv(const uint8_t* const bytes, std::size_t len) override;

	/// tcp_protocol::on_send
	void on_send(std::size_t len) override;

	/// tcp_protocol::on_error
	void on_error(const boost::system::error_code& ec) override;


	static void set_topic(topic::key_t key, resize_buffer::iterator& iter);

	static void set_length(packet::len_t len, resize_buffer::iterator& iter);

	static uint32_t get_length(resize_buffer::iterator& iter);

	static uint32_t get_topic(resize_buffer::iterator& iter);

	bool needs_to_modify(bits_packet::ptr m) const;

public:
	/// for packetiziation test only.  
	result call_recv_for_test(const uint8_t* const bytes, std::size_t len);

private:
	resize_buffer	recv_buf_;
	sequencer		sequencer_;
	checksum		checksum_;
	cipher			cipher_;
};

} // kernel
} // wise
