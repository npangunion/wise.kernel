#include "stdafx.h"

#include <wise/net/session.hpp>
#include <wise/net/network.hpp>
#include <wise/net/protocol/protocol_factory.hpp>
#include <wise/net/protocol/sys_messages.hpp>
#include <wise/base/logger.hpp>
#include <wise/base/exception.hpp>
#include <spdlog/fmt/fmt.h>

namespace wise
{

session::segment_buffer session::seg_buffer_accessor_;

session::session(const id& id, tcp::socket&& soc, bool accepted, const std::string& protocol)
	: socket_(std::move(soc))
	, id_(id)
	, accepted_(accepted)
	, protocol_name_(protocol)
{
}

void session::begin()
{
	// 다른 곳에 참조되지 않은 상태를 유지
	// thread-unsafe한 것들 참조 금지. 
	// service 함수들 호출 주의. (락 이슈들 발생 가능)

	remote_addr_ = fmt::format(
		"{0}:{1}",
		socket_.remote_endpoint().address().to_string(),
		socket_.remote_endpoint().port()
	);

	local_addr_ = fmt::format(
		"{0}:{1}",
		socket_.local_endpoint().address().to_string(),
		socket_.local_endpoint().port()
	);

	desc_ = fmt::format(
		"session/0x{:x}/remote:{}/local:{}", 
		get_id().get_value(),
		remote_addr_, 
		local_addr_
	);

	protocol_ = protocol_factory::inst().create(protocol_name_);

	if (!protocol_)
	{
		WISE_THROW("fail to create protocol");
		return;
	}

	protocol_->bind(this);

	WISE_DEBUG( "{0} protocol bound", get_desc() );

	if (network::cfg.set_tcp_no_delay)
	{
		socket_.set_option(tcp::no_delay(true));
	}

	if (network::cfg.set_tcp_linger)
	{
		socket_.set_option(asio::socket_base::linger(true, 0));
	}

	if (network::cfg.set_tcp_reuse_addr)
	{
		socket_.set_option(asio::socket_base::reuse_address(true));
	}

	WISE_DEBUG(
		"welcome! {}. sid: 0x{:x} local: {} remote: {}",
		accepted_ ? "accepted" : "connected", 
		get_id().value_, 
		local_addr_, 
		remote_addr_ 
	);

	request_recv();
}

session::~session()
{
	WISE_DEBUG("bye! {0}", get_desc());
}

session::result session::send(packet::ptr m)
{
	WISE_TRACE(
		"session send. message:{} session:0x{:x}",
		m->get_desc(), get_id().get_value()
	);

	send_queue_.push(m);

	begin_send_from_queue();

	return result(true, reason::success);
}

void session::close_from_application()
{
	WISE_DEBUG("close of {} requested from application", get_desc());

	close();
}

void session::close(const asio::error_code& ec)
{
	// 연결이 유효하면 끊고 소멸을 시도한다. 

	// close
	{
		std::lock_guard<lock_type> lock(session_mutex_);

		if (destroyed_)
		{
			WISE_ASSERT(!sending_ && !recving_);
			WISE_ASSERT(!socket_.is_open());
			return;
		}

		// close
		{
			if (socket_.is_open())
			{
				asio::error_code rec;

				socket_.shutdown(socket_.shutdown_both, rec);
				socket_.close(rec);

				WISE_DEBUG("{0} close", get_desc());
			}
		}

		// 소멸이 가능하면 소멸을 처리한다. 
		// - app -> close -> destroy 또는
		// - recv_comp -> error -> close -> destroy 또는 
		// - send_comp -> error -> close -> destroy 이다

		// session 락 걸고 호출

		// destroy

		if (sending_ || recving_) // wait both if any 
		{
			return;
		}

		WISE_DEBUG("{0} destroying...", get_desc());

		// notify
		destroyed_ = true;

		(void)protocol_->on_error(ec);

	}// locked

	// 여기 호출 할 때 내가 지워짐
	network::inst().destroy(get_id(), ec);
}

session::result session::bind_channel(uintptr_t pkey)
{
	if (channel_bound_ > 0)
	{
		WISE_INFO(
			"rebinding session. sid: 0x{:x}, channe: {}", 
			get_id().get_value(), channel_bound_
		);
	}

	channel_bound_ = pkey;

	WISE_TRACE(
		"session bound to a channel. sid: 0x{:x}, channel: {}", 
		get_id().get_value(), channel_bound_
	);

	return result(true, reason::success);
}

uintptr_t session::get_bound_channel() const
{
	return channel_bound_;
}

uint64_t session::bind_oid(uint64_t v)
{
	oid_bound_ = v;
	return oid_bound_;
}

uint64_t session::get_bound_oid() const
{
	return oid_bound_;
}

bool session::bind(const link& bp)
{
	WISE_TRACE(
		"session: 0x{:x} binds a new link from {}", 
		get_id().get_value(), bp.get_channel_key()
	);

	return link_binder_.bind(bp);
}

void session::unbind(link::suid_t suid)
{
	link_binder_.unbind(suid);
}

void session::post(packet::ptr m)
{
	link_binder_.post(m);
	network::post(m);
}

bool session::is_busy() const 
{
	std::lock_guard<lock_type> lock(session_mutex_);
	return sending_ || recving_;
}

session::result session::send(const uint8_t* const data, std::size_t len)
{
	WISE_RETURN_IF(!socket_.is_open(), result(false, reason::fail_socket_closed));

	// append 
	{
		std::lock_guard<lock_type> lock(send_mutex_);
		send_buffer_.append(data, len);
	}

	// 여기는 항상 성공한다. 
	return result(true, reason::success);
}

void session::error(const asio::error_code& ec)
{
	last_error_ = ec;

	WISE_DEBUG(
		"{0} error. reason: {1}",
		get_desc(), 
		ec.message()
	);

	log()->flush();

	close(ec);		// will call destroy
}

void session::error(const result& rc)
{
	WISE_DEBUG(
		"{0} error. reason: {1}",
		get_desc(),
		get_reason_desc(rc.value)
	);

	log()->flush();

	// ERROR_UNEXP_NET_ERR로 전달
	close(asio::error_code(0x3B, asio::system_category()));	
}

session::result session::request_recv()
{
	// check recv
	{
		std::lock_guard<lock_type> lock(session_mutex_);
		WISE_ASSERT(!recving_);
		if (recving_)
		{
			return result(false, reason::fail_session_already_recving);
		}

		// lock 안에서 체크해야 함
		WISE_RETURN_IF(!is_open(), result(false, reason::fail_socket_closed));

		recving_ = true;
	}

	WISE_TRACE("{} request recv", get_desc() );

	auto id = get_id();

	// request recv. 한번에 하나만 읽고 위에서 막히므로 락 필요 없음
	socket_.async_read_some(
		asio::buffer(recv_buf_.data(), recv_buf_.size()),
		[this, id](asio::error_code ec, std::size_t len) { this->on_recv_completed(ec, len, id); }
	);

	return result(true, reason::success);
}

session::result session::request_send()
{
	// check send
	{
		std::lock_guard<lock_type> session_lock(session_mutex_);
		WISE_ASSERT(sending_ == true);


		// lock 안에서 체크해야 함
		WISE_RETURN_IF(!is_open(), result(false, reason::fail_socket_closed));

		// check data available
		{
			std::lock_guard<lock_type> segs_lock(send_mutex_);

			if (send_buffer_.size() == 0)
			{
				return result(true, reason::success_session_no_data_to_send);
			}
		}
	}

	// get bufs and send. 한번에 하나만 보내고 위에서 막히므로 send_segs_mutex_만 사용
	{
		std::lock_guard<lock_type> lock(send_mutex_);

		WISE_ASSERT(send_buffer_.size() > 0);
		WISE_ASSERT(sending_segs_.empty());
		WISE_ASSERT(sending_bufs_.empty());

		sending_bufs_.clear();
		sending_segs_.clear();

		// rvo and move will be fine
		sending_segs_ = send_buffer_.transfer();
	
		// 락 벗어나면 바로 차는 경우 발생 
		WISE_ASSERT(send_buffer_.size() == 0);
	}

	WISE_ASSERT(!sending_segs_.empty());

	send_request_size_ = 0;

	for (auto& b : sending_segs_)
	{
		sending_bufs_.push_back(asio::buffer(b->get().data(), b->get().size()));
		send_request_size_ += b->get().size();
	}

	WISE_ASSERT(!sending_bufs_.empty());
	WISE_ASSERT(send_request_size_ > 0);

	WISE_TRACE( 
		"{0} request send. {1} bytes", 
		get_desc(), 
		send_request_size_ 
	);

	asio::async_write(
		socket_,
		sending_bufs_,
		[this](asio::error_code ec, std::size_t len) {
		this->on_send_completed(ec, len);
	});

	return result(true, reason::success);
}

void session::on_recv_completed(asio::error_code& ec, std::size_t len, const id& sid)
{
	WISE_UNUSED(sid);

	if (!ec)
	{
		WISE_ASSERT(len > 0);
		WISE_TRACE("session 0x{:x} recvd {} bytes", get_id().get_value(), len);

		auto rc = protocol_->on_recv(recv_buf_.data(), len);

		// protocol_->on_recv() 까지 처리하고 recving_을 풀어야 한다. 
		// 안 그러면 소켓이 지워질 수 있다.

		// locked
		{ 
			std::lock_guard<lock_type> lock(session_mutex_); 
			WISE_ASSERT(recving_); 
			recving_ = false; 
		}

		if (rc)
		{
			auto rc2 = request_recv();

			if (!rc2)
			{
				error(rc2);
			}
		}
		else
		{
			WISE_INFO(
				"session recv protocol error. id: {0}, reason: {1}",
				get_id().get_value(),
				rc.value
			);

			error(ec);
		}
	}
	else
	{
		// locked
		{
			std::lock_guard<lock_type> lock(session_mutex_);
			WISE_ASSERT(recving_);
			recving_ = false;
		}

		error(ec);
	}
}

void session::on_send_completed(asio::error_code& ec, std::size_t len)
{
	WISE_UNUSED(len);

	// release segs before try sending again
	{
		std::lock_guard<lock_type> lock(send_mutex_);
		WISE_ASSERT(!sending_segs_.empty());

		if (!ec)
		{
			WISE_ASSERT(send_request_size_ == len);
		}

		send_buffer_.release(sending_segs_);
		sending_segs_.clear();
		sending_bufs_.clear();
		send_request_size_ = 0;
	}

	// sending 상태로 큐에서 전송하면 패킷들을 sending_segs에 축적만 한다. 
	// send_from_queue에서 request_send()를 직접 호출한다. 
	// sending_을 푸는 것도 큐가 비었을 때만 아래 함수에서 한다. 
	send_from_queue();
}

void session::begin_send_from_queue()
{
	// lock and check
	{
		std::lock_guard<lock_type> lock(session_mutex_);

		if (sending_)
		{
			return;
		}

		if (send_queue_.empty())
		{
			return;
		}

		sending_ = true;
	}

	packet::ptr mp;

	send_queue_.pop(mp);

	auto rc = protocol_->send(mp);

	if (!rc)
	{
		WISE_ERROR(
			"send error. session: {}, error: {}", 
			get_desc(), 
			get_reason_desc(rc.value)
		);

		// locked
		{
			std::lock_guard<lock_type> lock(session_mutex_);
			sending_ = false;
		}

		return;
	}

	// 여기서 실제 보낸다. 
	request_send();
}

void session::send_from_queue()
{
	// lock and check
	{
		std::lock_guard<lock_type> lock(session_mutex_);
		WISE_ASSERT(sending_);

		if (send_queue_.empty())
		{
			sending_ = false;
			return;
		}
	}

	// 무한 반복하는 걸 방지하기 위해 시작할 때 가져온 
	// 개수만큼만 전송을 하고 다음 번에 보냄
	// unsafe_size는 정확하지는 않음
	auto count = send_queue_.unsafe_size();

	for (std::size_t i = 0; i < count; ++i)
	{
		packet::ptr mp;

		if (send_queue_.pop(mp))
		{
			// 패킷을 변형하여 바이너리 스트림을 만든다. send_buffer_에 쌓인다.
			auto rc = protocol_->send(mp);

			if (!rc)
			{
				WISE_ERROR(
					"send error. session: {}, error: {}",
					get_desc(),
					get_reason_desc(rc.value)
				);

				// locked
				{
					std::lock_guard<lock_type> lock(session_mutex_);
					sending_ = false;
				}

				error(rc);

				return;
			}
		}
		else
		{
			break;
		}
	}

	// 모아서 여기서 한번에 보낸다. 
	request_send();
}

} // wise
