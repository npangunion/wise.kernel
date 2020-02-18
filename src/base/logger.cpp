#include <pch.hpp>

#include <wise.kernel/base/logger.hpp>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/wincolor_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

namespace
{
void create_folder_from(const char* path);
}

namespace wise {
	namespace kernel {

		const char*		system_logger::name = "system";
		bool			system_logger::enable_console = true;
		unsigned int	system_logger::async_queue_size = 128 * 1024;
		unsigned int	system_logger::flush_interval_seconds = 2;
		const char*		system_logger::file_prefix = "logs/system.log";
		const char*		system_logger::log_pattern = "%^[%Y-%m-%d %H:%M:%S.%e][%t][%L] %v%$";
		bool			system_logger::retry_on_overflow = true;

		std::shared_ptr<spdlog::logger> system_logger::logger_;
		std::recursive_mutex system_logger::mutex_;
		std::atomic<bool> system_logger::initialized_ = false;

		void system_logger::init()
		{
			// 초기화 중 여러 곳에서 호출할 경우 블럭 시킴
			std::lock_guard<std::recursive_mutex> lock(mutex_);

			WISE_RETURN_IF(initialized_);

			initialized_ = true;

			spdlog::init_thread_pool(async_queue_size, 1);

			spdlog::set_pattern(log_pattern);
			spdlog::flush_every(std::chrono::seconds(flush_interval_seconds));

			create_folder_from(file_prefix);

			auto s1 = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();

			s1->set_color(spdlog::level::info, s1->GREEN | s1->BOLD);
			s1->set_color(spdlog::level::debug, s1->WHITE);
			s1->set_color(spdlog::level::trace, s1->CYAN);

			auto s2 = std::make_shared<spdlog::sinks::daily_file_sink<
				std::mutex,
				spdlog::sinks::daily_filename_calculator>
			>(file_prefix, 0, 0);

			s1->set_pattern(log_pattern);
			s2->set_pattern(log_pattern);

			std::vector<spdlog::sink_ptr> sinks{ s1, s2 };

			auto logger = std::make_shared<spdlog::async_logger>(
				name, sinks.begin(), sinks.end(),
				spdlog::thread_pool()
				);

			spdlog::register_logger(logger);

			get()->flush_on(spdlog::level::err);
		}
	}
} // wise::kernel

#include <filesystem>

using namespace std::filesystem;

namespace
{
	void create_folder_from(const char* apath)
	{
		path p(apath);
		path pp = p.remove_filename();

		create_directories(pp);
	}
}
