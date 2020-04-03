#pragma once

#include <string>

namespace wise {
namespace kernel {

enum class reason
{
	success,

	// general
	fail_null_pointer,
	fail_zero_size_data,
	fail_invalid_parameter,
	fail_not_implemented,

	// net
	fail_invalid_address,
	fail_acceptor_open,
	fail_acceptor_bind,
	fail_acceptor_listen,

	// session
	fail_session_already_recving,
	success_session_already_sending,
	success_session_no_data_to_send,
	fail_invalid_session,

	fail_socket_closed,

	// protocol 
	fail_null_message_pointer,
	fail_invalid_message_header,
	fail_protocol_not_added,
	fail_protocol_not_bound,
	fail_invalid_bits_message,
	fail_bits_message_not_registered,
	fail_bits_pack_error,
	fail_bits_unpack_error,
	fail_insufficient_unpack,
	fail_bits_max_packet_size,
	fail_invalid_message_sequence,
	fail_null_hash_function,
	fail_incorrect_checksum,
	fail_incorrect_checksum_length,

	// service and others
	fail_service_not_found,
	fail_handler_not_found,
};

/// get description from reason
std::string get_reason_desc(reason r);

} // kernel
} // wise
