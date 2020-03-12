#pragma once 

#include <wise/channel/channel.hpp>
#include <wise/net/protocol/protocol.hpp>
#include <wise/net/reason.hpp>
#include <wise/net/packet.hpp>
#include <wise/net/session_ref.hpp>
#include <wise/net/detail/buffer/segment_buffer.hpp>
#include <wise/net/detail/link_binder.hpp>
#include <wise/base/spinlock.hpp>
#include <wise/base/macros.hpp>
#include <wise/base/result.hpp>

#include <memory>
#include <mutex>

using namespace asio::ip;

namespace wise
{

/// socket wrapper to use with asio
/** 
 * 바이트 송수신 처리 세션. 
 * - 각 프로토콜 처리는 protocol 클래스에서 처리
 *
 */
class session final : public std::enable_shared_from_this<session>
{
public:

	using ptr = std::shared_ptr<session>;
	using wptr = std::weak_ptr<session>;
	using result = result<bool, reason>;
	using lock_type = spinlock;

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
	session(
		const id& id,
		tcp::socket&& soc,
		bool accepted,
		const std::string& protocol);

	/// clean up 
	virtual ~session();

	/// begin working 
	void begin();

	/// send through a protocol. call following send
	result send(packet::ptr m);

	/// close socket (shutdown and close) from application.
	void close_from_application();

	/// bind a channel to a session. 데이터로만 사용한다.
	result bind_channel(uintptr_t pkey);
	uintptr_t get_bound_channel() const;

	/// auth data
	uint64_t bind_oid(uint64_t v);
	uint64_t get_bound_oid() const;

	/// bind a link to link a channel
	bool bind(const link& bp);

	/// unbind
	void unbind(link::suid_t suid);

	/// 메세지 전달이 이 쪽을 거쳐야 브리지를 통해 채널로 전달됨
	/// network::post()를 같이 호출함
	void post(packet::ptr m);

	/// check 
	bool is_open() const
	{
		return socket_.is_open();
	}

	/// get internal id
	const id& get_id() const;

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
	const asio::error_code& get_last_error() const
	{
		return last_error_;
	}

private:
	/// send a packet to socket
	result send(const uint8_t* const data, std::size_t len);

	/// close socket (shutdown and close).
	void close(const asio::error_code& ec = asio::error::shut_down);

	/// 에러 처리 함수
	void error(const asio::error_code& ec);

	/// 자체 에러 
	void error(const result& rc);

	/// recv request
	result request_recv();

	/// send request
	result request_send();

	/// callback on recv
	void on_recv_completed(asio::error_code& ec, std::size_t len, const id& sid);

	/// callback on send 
	void on_send_completed(asio::error_code& ec, std::size_t len);

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
	std::vector<asio::const_buffer> sending_bufs_;

	asio::error_code				last_error_;
	bool							destroyed_ = false;

	std::atomic<uintptr_t>			channel_bound_ = 0;
	std::atomic<uint64_t>			oid_bound_ = 0;

	link_binder						link_binder_;
};

//
// TODO: 
// - per session packet rate control w/ policy
// - per session traffic control w/ policy
// - per packet rate control w/ policy
//

} // wise

#include "session.inl"

#include <unordered_map>

  // hash function for id
namespace std
{
template <>
struct hash<wise::session::id>
{
	std::size_t operator() (const wise::session::id& id) const
	{
		return std::hash<std::size_t>()(id.get_value());
	}
};
}
