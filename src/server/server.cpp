#include <pch.hpp>
#include <wise.kernel/server/server.hpp>

namespace wise
{
namespace kernel
{

server::server()
{

}

server::~server()
{

}

server::result server::start()
{
	// config
	{
		auto rc = load_config(); 
		WISE_RETURN_IF(!rc, result(false, error_code::fail_load_config));
	}

	// scheduler
	{
		auto rc = scheduler_.start(scheduler_cfg_);
		WISE_RETURN_IF(!rc, result(false, error_code::fail_start_task_scheduler));
	}

	id_generator_.setup(domain_);

	return result(true, error_code::success);
}

server::result server::run()
{
	scheduler_.schedule();

	return result(true, error_code::success);
}

void server::finish()
{
	scheduler_.finish();
}

bool server::load_config()
{
	// TODO: load configuration from json file

	scheduler_cfg_.runner_count = std::thread::hardware_concurrency();

	domain_ = 1; // юс╫ц

	return true;
}

} // kernel
} // wise
