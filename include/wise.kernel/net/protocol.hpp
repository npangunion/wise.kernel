#pragma once 

#include <wise.kernel/net/reason.hpp>
#include <wise.kernel/net/buffer/resize_buffer.hpp>

#include <wise.kernel/core/channel/channel.hpp>
#include <wise.kernel/core/result.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <shared_mutex>

namespace wise {
namespace kernel {

class packet;
using packet_ptr = std::shared_ptr<packet>;

/// abstract base class for protocols
/**
 * bind, unbind로 채널을 연결할 수 있다.
 * 수신한 메세지들은 연결된 모든 채널로 배부된다.
 */
class protocol : public std::enable_shared_from_this<protocol>
{
public:
	using ptr = std::shared_ptr<protocol>;
	using result = result<bool, reason>;
	using id_t = uint32_t;

public:
	/// constructor
	protocol()
		: id_(0)
		, mutex_()
		, channels_()
	{}

	/// destructor
	virtual ~protocol() {}

	/// send to a session after processing message
	virtual result send(packet_ptr m) = 0;

	bool bind(channel::ptr chan);

	void publish(packet_ptr m);

	void unbind(channel::ptr chan);

	void unbind(channel::key_t key);

	bool has_channel(channel::key_t key) const;

	void set_id(id_t id);

	id_t get_id() const;

private:
	using channel_map = std::map<channel::key_t, channel::ptr>;

private:
	id_t						id_;
	mutable std::shared_mutex	mutex_;
	channel_map					channels_;
};

inline
void protocol::set_id(id_t id)
{
	id_ = id;
}

inline
protocol::id_t protocol::get_id() const
{
	return id_;
}

} // kernel
} // wise
