#include <pch.hpp>
#include <wise.kernel/net/protocol.hpp>
#include <wise.kernel/net/packet.hpp>
#include <wise.kernel/core/logger.hpp>
#include <mutex>

namespace wise {
namespace kernel {

bool protocol::bind(channel::ptr chan)
{
	if (has_channel(chan->get_key()))
	{
		return true; // allow it
	}

	std::unique_lock<std::shared_mutex> lock(mutex_);
	channels_.insert(channel_map::value_type(chan->get_key(), chan));
	
	return true;
}

void protocol::publish(packet_ptr m)
{
	std::shared_lock<std::shared_mutex> lock(mutex_);

	for (auto& kv : channels_)
	{
		kv.second->publish(std::static_pointer_cast<message>(m));
	}
}

void protocol::unbind(channel::ptr chan)
{
	unbind(chan->get_key());
}

void protocol::unbind(channel::key_t key)
{
	std::unique_lock<std::shared_mutex> lock(mutex_);
	channels_.erase(key);
}

bool protocol::has_channel(channel::key_t key) const
{
	std::shared_lock<std::shared_mutex> lock(mutex_);
	return channels_.find(key) != channels_.end();
}

} // kernel 
} // wise