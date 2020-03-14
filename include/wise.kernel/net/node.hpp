#pragma once 

#include <wise.kernel/core/result.hpp>
#include <wise.kernel/net/reason.hpp>
#include <boost/asio.hpp>

#include <mutex>
#include <functional>

namespace wise {
namespace kernel {

/// provide asio::io_service
class node 
{
public:
	struct config
	{
		bool use_hardware_concurreny = true;
		int concurreny_level = 1;				/// core개수 사용하지 않을 경우
		bool enable_detail_log = false;			/// 자세한 로그 남길지 여부
	};

	using error_code = boost::system::error_code;
	using result = result<bool, reason>;

public:
	node(const config& _config);

	virtual ~node();

	/// start
	result start();

	/// finish
	void finish();

	/// check whether serivce is started
	bool is_running() const
	{
		return !stop_;
	}

	boost::asio::io_service& ios() 
	{ 
		return ios_; 
	}

protected:
	virtual result on_start() = 0;

	virtual void on_finish() = 0;

private:
	void run();

private:
	using threads = std::vector<std::thread>;

private:
	config					config_;
	boost::asio::io_service	ios_;
	std::atomic<bool>		stop_ = true;
	threads					threads_;
};

} // kernel
} // wise
