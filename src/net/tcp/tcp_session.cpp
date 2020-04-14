#include <pch.hpp>

#include <wise.kernel/net/tcp/tcp_session.hpp>
#include <wise.kernel/net/tcp/tcp_protocol.hpp>
#include <wise.kernel/net/tcp/tcp_node.hpp>
#include <wise.kernel/net/protocol_factory.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/exception.hpp>
#include <spdlog/fmt/fmt.h>

using namespace boost::system;

namespace wise {
namespace kernel {

tcp_session::tcp_session(tcp_protocol* proto, tcp::socket&& soc)
	: protocol_(proto)
	, socket_(std::move(soc))
	, last_error_(errc::make_error_code(errc::success))
{
}

void tcp_session::begin()
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
		"tcp_session. remote:{}/local:{}",
		remote_addr_,
		local_addr_
	);

	WISE_DEBUG("{0} protocol bound", get_desc());

	auto cfg = protocol_->get_node()->get_config();

	if (cfg.set_tcp_no_delay)
	{
		socket_.set_option(tcp::no_delay(true));
	}

	if (cfg.set_tcp_linger)
	{
		socket_.set_option(boost::asio::socket_base::linger(true, 0));
	}

	if (cfg.set_tcp_reuse_addr)
	{
		socket_.set_option(boost::asio::socket_base::reuse_address(true));
	}

	WISE_DEBUG(
		"welcome! {}. local: {} remote: {}",
		is_accepted() ? "accepted" : "connected",
		local_addr_,
		remote_addr_
	);

	request_recv();
}

tcp_session::~tcp_session()
{
	WISE_DEBUG("bye! {0}", get_desc());

	// requires close before destruction. tcp_node의 role.
	WISE_ASSERT(!is_busy());
	WISE_ASSERT(!socket_.is_open());
}

tcp_session::result tcp_session::send(const uint8_t* data, std::size_t len)
{
	WISE_RETURN_IF(!socket_.is_open(), result(false, reason::fail_socket_closed));

	// append 
	{
		std::lock_guard<lock_type> lock(send_mutex_);
		send_buffer_.append(data, len);
	}

	return request_send();
}

void tcp_session::disconnect()
{
	WISE_DEBUG("close of {} requested from application", get_desc());
	close();
}

void tcp_session::close()
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
				error_code rec;

				socket_.shutdown(socket_.shutdown_both, rec);
				socket_.close();

				WISE_DEBUG("{0} close", get_desc());
			}
		}

		// 소멸이 가능하면 소멸을 처리한다. 
		// - app -> close -> destroy 또는
		// - recv_comp -> error -> close -> destroy 또는 
		// - send_comp -> error -> close -> destroy 이다

		// tcp_session 락 걸고 호출

		// destroy

		if (sending_ || recving_) // wait both if any 
		{
			return;
		}

		WISE_DEBUG("{0} destroying...", get_desc());

		// notify
		destroyed_ = true;
	}// locked
}

bool tcp_session::is_accepted() const
{
	return protocol_->is_accepted();
}

bool tcp_session::is_busy() const
{
	std::lock_guard<lock_type> lock(session_mutex_);
	return sending_ || recving_;
}

void tcp_session::error(const error_code& ec)
{
	last_error_ = ec;

	WISE_DEBUG(
		"{0} error. reason: {1}",
		get_desc(),
		ec.message()
	);

	log()->flush();

	close();		

	protocol_->on_error(ec);
}

void tcp_session::error(const result& rc)
{
	WISE_DEBUG(
		"{0} error. reason: {1}",
		get_desc(),
		get_reason_desc(rc.value)
	);

	log()->flush();

	close();

	// ERROR_UNEXP_NET_ERR로 전달
	error_code ec(0x3B, boost::system::system_category());
	protocol_->on_error(ec);
}

tcp_session::result tcp_session::request_recv()
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

	WISE_TRACE("{} request recv", get_desc());

	// request recv. 한번에 하나만 읽고 위에서 막히므로 락 필요 없음
	socket_.async_read_some(
		boost::asio::buffer(recv_buf_.data(), recv_buf_.size()),
		[this](error_code ec, std::size_t len) { this->on_recv_completed(ec, len); }
	);

	return result(true, reason::success);
}

tcp_session::result tcp_session::request_send()
{
	// check send
	{
		std::lock_guard<lock_type> session_lock(session_mutex_);
		WISE_RETURN_IF(sending_, result(true, reason::success_session_already_sending));

		// lock 안에서 체크해야 함
		WISE_RETURN_IF(!is_open(), result(false, reason::fail_socket_closed));

		// 버퍼 체크
		{
			std::lock_guard<lock_type> segs_lock(send_mutex_);

			// check data available
			if (send_buffer_.size() == 0)
			{
				return result(true, reason::success_session_no_data_to_send);
			}
		}

		// return과 설정은 같이 체크해야 한다.
		sending_ = true;
	}

	// 전송 처리
	{
		std::lock_guard<lock_type> segs_lock(send_mutex_);

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
		sending_bufs_.push_back(boost::asio::buffer(b->get().data(), b->get().size()));
		send_request_size_ += b->get().size();
	}

	WISE_ASSERT(!sending_bufs_.empty());
	WISE_ASSERT(send_request_size_ > 0);

	WISE_TRACE(
		"{0} request send. {1} bytes",
		get_desc(),
		send_request_size_
	);

	boost::asio::async_write(
		socket_,
		sending_bufs_,
		[this](error_code ec, std::size_t len) {
			this->on_send_completed(ec, len);
		});

	return result(true, reason::success);
}

void tcp_session::on_recv_completed(error_code& ec, std::size_t len)
{
	if (!ec)
	{
		WISE_ASSERT(len > 0);
		WISE_TRACE("tcp_session recvd {} bytes", len);

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
			WISE_INFO( "tcp_session recv protocol error. reason: {1}", rc.value );

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

void tcp_session::on_send_completed(error_code& ec, std::size_t len)
{
	WISE_UNUSED(len);

	// release segs before try sending again
	{
		std::lock_guard<lock_type> lock(send_mutex_);
		WISE_ASSERT(!sending_segs_.empty());

		if (!ec)
		{
			WISE_ASSERT(send_request_size_ == len);
			protocol_->on_send(len);
		}

		send_buffer_.release(sending_segs_);
		sending_segs_.clear();
		sending_bufs_.clear();
		send_request_size_ = 0;
	}

	// clear sending 
	{
		std::lock_guard<lock_type> sl(session_mutex_);
		sending_ = false;
	}

	request_send();
}

} // kernel
} // wise

