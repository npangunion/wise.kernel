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

	WISE_INFO("asio node finishing...");

	on_finish();

	// 종료 중에 정리를 위해 ios는 실행 필요
	stop_ = true;

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

	WISE_INFO("asio node finished");
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
