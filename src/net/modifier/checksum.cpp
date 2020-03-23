#include <pch.hpp>
#include <wise.kernel/net/modifier/checksum.hpp>
#include <botan/botan.h>

namespace wise {
namespace kernel {

checksum::checksum(std::size_t header_length)
	: modifier()
	, header_length_(header_length)
{
}

modifier::result checksum::on_recv(
	resize_buffer& buf,
	std::size_t msg_offset,
	std::size_t msg_len,
	std::size_t& new_len
)
{
	new_len = msg_len;

	// WISE_EXPECT(msg_offset >= 0 ); // by type
	// WISE_EXPECT(msg_len >= 0); // by type

	WISE_EXPECT(buf.size() >= msg_len);
	WISE_EXPECT(msg_len >= header_length_);

	// no payload 
	WISE_RETURN_IF(
		msg_len == header_length_,
		result(true, reason::success)
	);

	// for thread-safey, create TLS hash function 
	// - windows, android, ios, osx, linux supports it
	static thread_local auto hash = Botan::HashFunction::create("CRC32");
	WISE_RETURN_IF(!hash, result(false, reason::fail_null_hash_function));

	hash->clear();

	hash->update(
		buf.data() + msg_offset + header_length_,
		msg_len - checksum_size - header_length_
	);

	uint8_t crc[checksum_size];
	hash->final(crc);

	auto rc = std::memcmp(
		buf.data() + msg_offset + msg_len - checksum_size,
		crc,
		checksum_size
	);

	if (rc != 0)
	{
		return result(false, reason::fail_incorrect_checksum);
	}

	// WISE_ENSURE(sender checksum == receiver checksum);

	new_len = msg_len - checksum_size;

	return result(true, reason::success);
}

modifier::result checksum::on_send(
	resize_buffer& buf,
	std::size_t msg_offset,
	std::size_t msg_len
)
{
	WISE_EXPECT(msg_len >= header_length_);

	// no payload 
	WISE_RETURN_IF(
		msg_len == header_length_,
		result(true, reason::success)
	);

	WISE_EXPECT(msg_len > header_length_);

	// for thread-safey, create TLS hash function 
	static thread_local auto hash = Botan::HashFunction::create("CRC32");
	WISE_RETURN_IF(!hash, result(false, reason::fail_null_hash_function));

	hash->clear();

	hash->update(
		buf.data() + msg_offset + header_length_,
		msg_len - header_length_
	);

	uint8_t crc[checksum_size];
	hash->final(crc);

	buf.append("abcd", checksum_size); 

	std::memcpy(buf.data() + msg_offset + msg_len, crc, checksum_size);

	update_length_field(buf, msg_offset, msg_len + checksum_size);

	return result(true, reason::success);
}

} // kernel
} // wise
