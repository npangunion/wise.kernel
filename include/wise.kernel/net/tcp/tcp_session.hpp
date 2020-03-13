#pragma once 

#include <wise.kernel/net/protocol.hpp>
#include <wise.kernel/net/reason.hpp>
#include <wise.kernel/net/packet.hpp>
#include <wise.kernel/net/buffer/segment_buffer.hpp>
#include <wise.kernel/core/spinlock.hpp>
#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/core/result.hpp>

#include <memory>
#include <mutex>

using namespace boost::asio::ip;

namespace wise {
namespace kernel {

/// socket wrapper to use with asio
/**
 * 바이트 송수신 처리 세션.
 * - 각 프로토콜 처리는 protocol 클래스에서 처리
 *
 */
class tcp_session final
{
public:
	using result = result<bool, reason>;
	using lock_type = spinlock;
	using error_code = boost::system::error_code;

	friend class protocol; // to use send and error

public:
	union id
	{
		using full_t = uint32_t;
		using seq_t = uint16_t;
		using index_t = uint16_t;

		full_t value_ = 0;

		struct full
		{
			seq_t seq_;
			index_t index_;
		} full_;

		explicit id(full_t value = 0);
		id(seq_t seq, index_t index);

		/// seq_ > 0 
		bool is_valid() const;

		const full_t get_value() const;
		const seq_t get_seq() const;
		const index_t get_index() const;

		bool operator==(const id& rhs) const;
		bool operator!=(const id& rhs) const;
		bool operator < (const id& rhs) const;
	};

	using sid_t = id::full_t;

public:
	/// setup 
	tcp_session(
		const id& id,
		tcp::socket&& soc,
		bool accepted,
		const std::string& protocol);

	/// clean up 
	virtual ~tcp_session();

	/// begin working 
	void begin();

	/// send through a protocol. call following send
	result send(packet::ptr m);

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
	bool is_accepted() const
	{
		return accepted_;
	}

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
	/// send a packet to socket
	result send(const uint8_t* const data, std::size_t len);

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
	void on_recv_completed(error_code& ec, std::size_t len, const id& sid);

	/// callback on send 
	void on_send_completed(error_code& ec, std::size_t len);

	/// 패킷 전송 큐에서 하나 보내서 전송 시작
	void begin_send_from_queue();

	/// IO 쓰레드에서 큐에 있는 것들 모두 모아서 보냄
	void send_from_queue();

private:
	using segment_buffer = segment_buffer<32 * 1024>;
	using seg = typename segment_buffer::seg;

	static segment_buffer			seg_buffer_accessor_;

	tcp::socket						socket_;
	id								id_;
	bool							accepted_ = false;
	std::string						protocol_name_;
	protocol::ptr					protocol_;
	std::string						local_addr_;
	std::string						remote_addr_;
	std::string						desc_;

	mutable lock_type				session_mutex_;
	bool							recving_ = false; /// sending과 recving은 항상 session_mutex_ 걸고 처리
	bool							sending_ = false;
	std::array<uint8_t, 32 * 1024>	recv_buf_;			// 32K를 넘으면 느려지는 현상이 x64, i7 장비에서 발생. 확인 필요

	lock_type						send_mutex_;
	concurrent_queue<packet::ptr>	send_queue_;
	segment_buffer					send_buffer_;
	std::size_t						send_request_size_ = 0;
	std::vector<seg*>				sending_segs_;
	std::vector<boost::asio::const_buffer> sending_bufs_;

	error_code						last_error_;
	bool							destroyed_ = false;
};

//
// TODO: 
// - per session packet rate control w/ policy
// - per session traffic control w/ policy
// - per packet rate control w/ policy
//

} // kernel
} // wise


