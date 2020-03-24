#include <pch.hpp>
#include <wise.kernel/net/node.hpp>
#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/core/logger.hpp>

namespace wise {
namespace kernel {


node::node(const config& _config)
	: config_(_config)
	, ios_()
{
}

node::~node()
{
	finish();
}

node::result node::start()
{
	auto n = config_.concurreny_level;

	if (config_.use_hardware_concurreny)
	{
		n = std::thread::hardware_concurrency();
	}

	WISE_ASSERT(n > 0);
	WISE_ASSERT(stop_);

	stop_ = false;

	threads_.resize(n);

	for (int i = 0; i < n; ++i)
	{
		auto thread = std::thread([this]() {this->run(); });
		threads_[i].swap(thread);
	}

	WISE_INFO("asio node started");

	return on_start();
}

void node::finish()
{
	WISE_RETURN_IF(stop_);
	stop_ = true;

	on_finish();

	// post to all threads
	for (auto& t : threads_)
	{
		WISE_UNUSED(t);
		ios_.stop();
	}

	// wait all threads
	for (auto& t : threads_)
	{
		t.join();
	}
}


bool node::bind(channel::ptr chan)
{
	if (has_channel(chan->get_key()))
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		auto iter = channels_.find(chan->get_key());

		if (iter->second == chan)
		{
			return true; // allow it
		}

		WISE_THROW("illegal bind of a different channel with same key");
	}

	std::unique_lock<std::shared_mutex> lock(mutex_);
	channels_.insert(channel_map::value_type(chan->get_key(), chan));

	return true;
}

void node::publish(packet::ptr m)
{
	std::shared_lock<std::shared_mutex> lock(mutex_);

	for (auto& kv : channels_)
	{
		kv.second->publish(std::static_pointer_cast<message>(m));
	}
}

void node::unbind(channel::ptr chan)
{
	unbind(chan->get_key());
}

void node::unbind(channel::key_t key)
{
	std::unique_lock<std::shared_mutex> lock(mutex_);
	channels_.erase(key);
}

bool node::has_channel(channel::key_t key) const
{
	std::shared_lock<std::shared_mutex> lock(mutex_);
	return channels_.find(key) != channels_.end();
}

void node::run()
{
	while (!stop_)
	{
		auto n = ios_.run();

		if (n == 0)
		{
			// 처리 한 요청이 없으면 잠시 쉰다. 
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

} // kernel
} // wise
