#include <pch.hpp>
#include <wise.kernel/net/reason.hpp>

std::string wise::get_reason_desc(reason r)
{
	static std::string desc[] = { 
		"success",

		// general
		"fail_null_pointer",
		"fail_zero_size_data",
		"fail_invalid_parameter",
		"fail_not_implemented",

		// net
		"fail_invalid_session_index",
		"fail_invalid_address",
		"fail_acceptor_open",
		"fail_acceptor_bind",
		"fail_acceptor_listen",

		"fail_session_already_recving",
		"success_session_already_sending",
		"success_session_no_data_to_send",

		"fail_socket_closed",

		// protocol 
		"fail_null_message_pointer",
		"fail_invalid_message_header",
		"fail_protocol_not_added",
		"fail_invalid_zen_message",
		"fail_zen_message_not_registered",
		"fail_zen_pack_error",
		"fail_zen_unpack_error",
		"fail_zen_max_packet_size",
		"fail_invalid_message_sequence",
		"fail_null_hash_function",
		"fail_incorrect_checksum",
		"fail_incorrect_checksum_length",

		// service and others
		"fail_service_not_found",
	};

	return desc[static_cast<int>(r)];
}
