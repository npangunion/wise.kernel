#pragma once 

#include <wise.kernel/net/protocol.hpp>
#include <wise.kernel/net/reason.hpp>
#include <wise.kernel/net/packet.hpp>
#include <wise.kernel/net/buffer/segment_buffer.hpp>
#include <wise.kernel/core/spinlock.hpp>
#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/core/result.hpp>

#include <boost/asio.hpp>
#include <memory>
#include <mutex>

using namespace boost::asio::ip;

namespace wise {
namespace kernel {

/// tcp socket wrapper to use with asio
/**
 * 바이트 송수신 처리 세션.
 * - 각 프로토콜 처리는 protocol 클래스에서 처리
 * - protocol 클래스에서 처리하고 send() 호출
 */
class tcp_session final
{
public:
	using result = result<bool, reason>;
	using lock_type = spinlock;
	using error_code = boost::system::error_code;

	friend class tcp_protocol; 

public:
	/// setup 
	tcp_session(
		tcp_protocol* proto,
		tcp::socket&& soc);

	/// clean up 
	virtual ~tcp_session();

	/// begin working 
	void begin();

	/// send a packet to socket
	result send(const uint8_t* data, std::size_t len);

	/// close socket (shutdown and close) from application.
	void disconnect();

	/// check 
	bool is_open() const
	{
		return socket_.is_open();
	}

	/// local endpoint address
	const std::string& get_local_addr() const
	{
		return local_addr_;
	}

	/// remote endpoint address
	const std::string& get_remote_addr() const
	{
		return remote_addr_;
	}

	const std::string& get_desc() const
	{
		return desc_;
	}

	/// accept 여부
	bool is_accepted() const;

	bool is_busy() const;

	/// internal use only.  
	lock_type& get_session_lock()
	{
		return session_mutex_;
	}

protected:
	// get last error (for debug / test purpose)
	const error_code& get_last_error() const
	{
		return last_error_;
	}

private:

	/// close socket (shutdown and close).
	void close(const error_code& ec);

	/// 에러 처리 함수
	void error(const error_code& ec);

	/// 자체 에러 
	void error(const result& rc);

	/// recv request
	result request_recv();

	/// send request
	result request_send();

	/// callback on recv
	void on_recv_completed(error_code& ec, std::size_t len);

	/// callback on send 
	void on_send_completed(error_code& ec, std::size_t len);

private:
	using segment_buffer = segment_buffer<32 * 1024>;
	using seg = typename segment_buffer::seg;

	static segment_buffer			seg_buffer_accessor_;

	tcp_protocol*					protocol_ = nullptr;
	tcp::socket						socket_;
	std::string						local_addr_;
	std::string						remote_addr_;
	std::string						desc_;

	mutable lock_type				session_mutex_;
	bool							recving_ = false; /// sending과 recving은 항상 session_mutex_ 걸고 처리
	bool							sending_ = false;
	std::array<uint8_t, 32 * 1024>	recv_buf_;			// 32K를 넘으면 느려지는 현상이 x64, i7 장비에서 발생. 확인 필요

	lock_type						send_mutex_;
	segment_buffer					send_buffer_;
	std::size_t						send_request_size_ = 0;
	std::vector<seg*>				sending_segs_;
	std::vector<boost::asio::const_buffer> sending_bufs_;

	error_code						last_error_;
	bool							destroyed_ = false;
};

} // kernel
} // wise


