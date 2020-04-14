#include <pch.hpp>
#include <wise.kernel/core/channel/channel.hpp>
#include <wise.kernel/core/channel/channel_map.hpp>

namespace wise {
namespace kernel {

channel::ptr channel::create(const key_t& key, const config& cfg)
{
	return channel_map::get().create(key, cfg);
}

channel::ptr channel::find(const key_t& key)
{
	return channel_map::get().find(key);
}

channel::ptr channel::find(uintptr_t pkey)
{
	return channel_map::get().find_from_addr(pkey);
}

bool channel::destroy(const key_t& key)
{
	return channel_map::get().destroy(key);
}

channel::channel(const key_t& key, const config& cfg)
	: key_(key)
	, config_(cfg)
	, map_(*this)
{
}

channel::~channel()
{
	clear();
}

std::size_t channel::publish(message::ptr m)
{
	q_.push(m);

	auto count = map_.post(m, sub::mode::immediate);

	stat_.total_immediate_post_count += count;

	return count;
}

std::size_t channel::publish(const topic& topic, message::ptr m)
{
	q_.push(m);

	auto count = map_.post(topic, m, sub::mode::immediate);

	stat_.total_immediate_post_count += count;

	return count;
}

std::size_t channel::execute()
{
	message::ptr m;

	std::size_t count = 0;

	simple_tick loop_tick;

	while (q_.pop(m))
	{
		simple_tick tick;

		count += map_.post(m, sub::mode::delayed);

		if (tick.elapsed() > config_.time_to_log_when_post_time_over)
		{
			WISE_INFO(
				"channel:{}, post: 0x{:x}, time: {} ms", 
				key_, 
				m->get_topic().get_key(), 
				tick.elapsed());
		}

		if (count > config_.loop_post_limit)
		{
			break;
		}
	}

	stat_.last_post_count = count;
	stat_.total_post_count += stat_.last_post_count;

	if (loop_tick.elapsed() > config_.time_to_log_when_post_loop_time_over)
	{
		WISE_INFO(
			"channel: {}, post loop: {} ms, count: {}", 
			key_, 
			loop_tick.elapsed(), 
			count);
	}

	return count;
}

sub::key_t channel::subscribe(const topic& topic, cond_t cond, cb_t cb, sub::mode mode)
{
	return map_.subscribe(topic, cond, cb, mode);
}

sub::key_t channel::subscribe(const topic& topic, cb_t cb, sub::mode mode)
{
	return map_.subscribe(topic, cb, mode);
}

bool channel::unsubscribe(sub::key_t key)
{
	return map_.unsubscribe(key);
}

void channel::clear()
{
	message::ptr m;

	while (q_.pop(m))
	{
		if (config_.log_remaining_messages_on_exit)
		{
			WISE_INFO("channel:{}. clear message:0x{:x}", key_, m->get_topic().get_key());
		}
	}

	map_.clear();
}

std::size_t channel::get_queue_size() const
{
	return q_.unsafe_size();
}

} //kernel
} // wise
