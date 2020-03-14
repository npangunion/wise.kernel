#include "stdafx.h"
#include <wise/net/protocol/zen/zen_protocol.hpp>
#include <wise/net/protocol/zen/zen_factory.hpp>
#include <wise/net/protocol/zen/zen_packer.hpp>
#include <wise/net/network.hpp>
#include <wise/base/exception.hpp>
#include <wise/base/logger.hpp>
#include <wise/base/macros.hpp>

namespace wise
{

/// static configuration info
zen_protocol::config zen_protocol::cfg;

zen_protocol::zen_protocol()
	: checksum_(zen_message::header_length)
	, cipher_(zen_message::header_length)
{
	if (cfg.enable_loopback)
	{
		on_bind();
	}
}

zen_protocol::~zen_protocol()
{
}

void zen_protocol::on_bind()
{
	if (!cfg.enable_loopback)
	{
		WISE_ENSURE(get_session());
	}

	sequencer_.bind(this);
	checksum_.bind(this);
	cipher_.bind(this);
}

protocol::result zen_protocol::send(packet::ptr m)
{
	WISE_EXPECT(m);
	WISE_RETURN_IF(!m, result(false, reason::fail_null_message_pointer));

	auto mp = std::static_pointer_cast<zen_message>(m);

	if (!mp)
	{
		WISE_CRITICAL(
			"zen_protocol expects zen_message to send. topic: {}/{}",
			topic::get_desc(m->get_topic())
		);

		return result(false, reason::fail_invalid_zen_message);
	}

	// TLS로 쓰레드별로 하나만 만듦.
	static thread_local std::unique_ptr<resize_buffer> pbuf(new resize_buffer());

	pbuf->rewind();

	auto rc = pack(mp, *pbuf);
	WISE_RETURN_IF(!rc, rc);

	return send_final(mp, *pbuf, pbuf->size());
}

protocol::result zen_protocol::pack(zen_message::ptr mp, resize_buffer& buf)
{
	uint32_t pad = 0xCAFEABBA;

	buf.append(&pad, sizeof(packet::len_t));	// length field
	buf.append(&pad, sizeof(topic::key_t));		// topic field

	zen_packer packer(buf);

	bool rc = mp->pack(packer); // message의 pack할 때 토픽을 포함

	if (!rc)
	{
		WISE_ERROR("failed to pack. topic:  0x{:x}", mp->get_topic().get_key());
		return result(false, reason::fail_zen_pack_error);
	}

	// size는 length와 topic 길이를 포함
	packet::len_t size = static_cast<packet::len_t>(buf.size());
	WISE_ASSERT(size >= zen_message::header_length);

	auto iter = buf.begin();

	set_length(size, iter);
	set_topic(mp->get_topic().get_key(), ++iter);

	return result(true, reason::success);
}

protocol::result zen_protocol::send_final(
	zen_message::ptr mp,
	resize_buffer& buf,
	std::size_t len
)
{
	if (!needs_to_modify(mp))
	{
		if (cfg.enable_loopback)
		{
			// 테스트 모드일 경우 
			return on_recv(buf.data(), len);
		}

		// else

		return protocol::send(buf.data(), len);
	}

	return send_modified(mp, buf, len);
}


protocol::result zen_protocol::send_final(
	zen_message::ptr mp, 
	const uint8_t* const data, 
	std::size_t len
)
{
	if (!needs_to_modify(mp))
	{
		if (cfg.enable_loopback)
		{
			// 테스트 모드일 경우 
			return on_recv(data, len);
		}

		// else

		return protocol::send(data, len);
	}

	// TODO: TLS buffer
	resize_buffer buf; 

	buf.append(data, len);

	return send_modified(mp, buf, len);
}

protocol::result zen_protocol::send_modified(
	zen_message::ptr mp,
	resize_buffer& buf,
	const std::size_t len
)
{
	// 여기에 오면 buf는 공유되지 않은 깨끗한 mp의 버퍼이다. 
	WISE_EXPECT(buf.size() == len);

	if (cfg.enable_sequence && mp->enable_sequence)
	{
		auto rc = sequencer_.on_send(buf, 0, buf.size());

		WISE_RETURN_IF(!rc, rc);
	}

	if (cfg.enable_checksum && mp->enable_checksum)
	{
		auto rc = checksum_.on_send(buf, 0, buf.size());

		WISE_RETURN_IF(!rc, rc);
	}

	if (cfg.enable_cipher && mp->enable_cipher)
	{
		auto rc = cipher_.on_send(buf, 0, buf.size());

		WISE_RETURN_IF(!rc, rc);
	}

	if (cfg.enable_loopback)
	{
		// 테스트 모드일 경우 
		return on_recv(buf.data(), buf.size());
	}

	WISE_ASSERT(buf.size() >= len);

	if (buf.size() > len)
	{
		auto iter = buf.begin();
		packet::len_t size = static_cast<packet::len_t>(buf.size());

		set_length(size, iter);
	}

	return protocol::send(buf.data(), buf.size());
}

protocol::result zen_protocol::on_recv(
	const uint8_t* const bytes, 
	std::size_t len)
{
	WISE_EXPECT(bytes);
	WISE_EXPECT(len > 0);
	WISE_RETURN_IF(!bytes, result(false, reason::fail_null_pointer));
	WISE_RETURN_IF(len == 0, result(false, reason::fail_zero_size_data));

	recv_buf_.append(bytes, len);

	auto iter = recv_buf_.begin();

	auto remained_len = recv_buf_.end() - iter;
	std::size_t processed_len = 0;

	while (remained_len >= zen_message::header_length)
	{
		auto msg_len = get_length(iter);	// 메세지 길이는 암호화 관련 변경을 포함

		WISE_ASSERT(msg_len >= zen_message::header_length);

		if (msg_len > cfg.max_packet_length)
		{
			WISE_ERROR(
				"max packet size. len: {}", msg_len
			);

			return result(false, reason::fail_zen_max_packet_size);
		}

		if (remained_len < msg_len)
		{
			break;
		}

		auto pic = get_topic(iter);			// forward iter while getting topic 
		auto tp = topic(pic);

		auto mp = ZEN_MSG_CREATE(pic);

		if (!mp)
		{
			WISE_ERROR(
				"zen_message for topic[{}:{}:{}] not registered",
				tp.get_category(), tp.get_group(), tp.get_type()
			);

			return result(false, reason::fail_zen_message_not_registered); // close
		}

		std::size_t final_len = 0;

		// 암호화 관련 처리
		auto rc = recv_modified(mp, recv_buf_, processed_len, msg_len, final_len);

		if (!rc)
		{
			WISE_ERROR(
				"fail to modify while recv. topic: {}. error:{} ",
				topic::get_desc(tp), rc.value 
			);

			return rc; // close
		} 

		// 메세지 내용 가져오기
		uint32_t pad = 0;

		// forward read position
		recv_buf_.read((uint8_t*)&pad, sizeof(packet::len_t));
		recv_buf_.read((uint8_t*)&pad, sizeof(topic::key_t));

		zen_packer packer(recv_buf_);

		auto res = mp->unpack(packer);

		// TODO: 메세지 길이보다 unpack에서 더 사용하면 에러 처리가 필요하다. 
		// final_len을 고려해야 한다. 
		// 

		if (!res)
		{
			WISE_ERROR("zen_message unpack error. topic: ", topic::get_desc(tp));
			return result(false, reason::fail_zen_unpack_error);
		}

		if (cfg.enable_loopback)
		{
			// 테스트에서 글로벌 채널로는 전달 된다고 가정
			network::post(mp);
		}
		else
		{
			WISE_ASSERT(get_session());

			mp->sid = get_session()->get_id().get_value();

			// 세션의 채널로 전달
			get_session()->post(mp);
		}

		// 다음 처리를 위한 정리 
		auto payload_len = msg_len - zen_message::header_length;

		processed_len += msg_len;
		iter += payload_len;		
		remained_len = recv_buf_.end() - iter;
	}

	if (processed_len > 0)
	{
		recv_buf_.pop_front(processed_len);
	}

	return result(true, reason::success);
}


protocol::result zen_protocol::recv_modified(
	zen_message::ptr mp,
	resize_buffer& buf,
	std::size_t msg_pos,
	std::size_t msg_len,
	std::size_t& final_len
)
{
	std::size_t new_len = msg_len;

	if (cfg.enable_cipher && mp->enable_cipher)
	{
		auto rc = cipher_.on_recv(buf, msg_pos, msg_len, new_len);

		WISE_RETURN_IF(!rc, rc);

		msg_len = new_len;
	}

	if (cfg.enable_checksum && mp->enable_checksum)
	{
		auto rc = checksum_.on_recv(buf, msg_pos, msg_len, new_len);

		WISE_RETURN_IF(!rc, rc);

		msg_len = new_len;
	}

	if (cfg.enable_sequence && mp->enable_sequence)
	{
		auto rc = sequencer_.on_recv(buf, msg_pos, msg_len, new_len);

		WISE_RETURN_IF(!rc, rc);

		msg_len = new_len;
	}

	final_len = new_len;

	return result(true, reason::success);
}


void zen_protocol::on_send(std::size_t len)
{
	WISE_UNUSED(len); 
	
	// 특별히 할 일은 없다
}

void zen_protocol::on_error(const asio::error_code& ec)
{
	WISE_UNUSED(ec);

	// 특별히 할 일이 없다.
}

protocol::result zen_protocol::on_recv_to_test(
	const uint8_t* const bytes,
	std::size_t len
)
{
	// wise purpose only

	return on_recv(bytes, len);
}

void zen_protocol::set_topic(topic::key_t key, resize_buffer::iterator& ri) 
{
	*ri = key & 0x000000FF;
	++ri; *ri = key >> 8 & 0x000000FF;
	++ri; *ri = key >> 16 & 0x000000FF;
	++ri; *ri = key >> 24 & 0x000000FF;
}

void zen_protocol::set_length(packet::len_t len, resize_buffer::iterator& ri) 
{
	*ri = len & 0x000000FF;
	++ri; *ri = len >> 8 & 0x000000FF;
	++ri; *ri = len >> 16 & 0x000000FF;
	++ri; *ri = len >> 24 & 0x000000FF;
}

uint32_t zen_protocol::get_length(resize_buffer::iterator& ri) 
{
	uint32_t len = 0;

	len = *ri;				++ri;
	len |= (*ri << 8);		++ri;
	len |= (*ri << 16);		++ri;
	len |= (*ri << 24);		++ri;

	return len;
}

uint32_t zen_protocol::get_topic(resize_buffer::iterator& ri) 
{
	uint32_t topic = 0;

	topic = *ri;			++ri;
	topic |= (*ri << 8);	++ri;
	topic |= (*ri << 16);	++ri;
	topic |= (*ri << 24);	++ri;

	return topic;
}

protocol::result zen_protocol::call_recv_for_test(
	const uint8_t* const bytes, 
	std::size_t len
)
{
	return on_recv(bytes, len);
}

bool zen_protocol::needs_to_modify(zen_message::ptr m) const
{
	return
		(cfg.enable_cipher		&& m->enable_cipher) ||
		(cfg.enable_checksum	&& m->enable_checksum) ||
		(cfg.enable_sequence	&& m->enable_sequence);
}

} // wise
