#include "stdafx.h"
#include "idl_context.h"
#include "idl_main.h"
#include <wise/base/macros.hpp>
#include <wise/base/logger.hpp>

idl_context& idl_context::inst()
{
	static idl_context inst_;

	return inst_;
}

int idl_context::start(const config& _config)
{
	if (started_)
	{
		WISE_ERROR("Parsing started already.");

		return -1;
	}

	config_ = _config;

	setup_config();

	WISE_RETURN_IF(config_.main_file.empty(), -1);

	started_ = true;

	int rc = 0;

	fstack_.push(file_state{ config_.main_file, false });

	// parsing phase 
	rc = parse();

	WISE_RETURN_IF(rc < 0, rc);

	// generation phase
	rc = generate();

	return  rc;
}

void idl_context::push_program(idl_program::ptr p)
{
	WISE_TRACE("Push program {}", p->get_path());

	pstack_.push(p);
}

void idl_context::push_file(const std::string& path)
{
	WISE_TRACE("Push file {}", path);

	auto iter = states_.find(path);

	// 포함 과정에서 여러 번 포함 될 수 있다. 
	WISE_RETURN_IF(iter != states_.end());

	if (iter == states_.end())
	{
		states_[path] = file_state{ path, false };

		fstack_.push(states_[path]);
	}
}

int idl_context::parse()
{
	while (!fstack_.empty())
	{
		auto f = fstack_.top();

		fstack_.pop();

		auto iter = states_.find(f.path);

		if (iter == states_.end() || !iter->second.processed)
		{
			auto rc = ::parse_file(f.path);

			if (rc != 0)
			{
				WISE_ERROR("Fail to parse file: {}", f.path);

				return -1;
			}

			f.processed = true;

			states_[f.path] = f;
		}
		else
		{
			WISE_TRACE("Met a processed file: {}", f.path);
		}
	}

	return 0;
}

int idl_context::generate()
{
	while (!pstack_.empty())
	{
		auto p = pstack_.top();

		pstack_.pop();

		auto rc = ::generate_program(p);

		if (!rc)
		{
			if (!config_.continue_generation_on_error)
			{
				WISE_ERROR(
					"Exit on generation failure on file: {}", 
					p->get_path() 
				);

				return -1;
			}
			else
			{
				WISE_ERROR(
					"Continue generation after failure on file: {}", 
					p->get_path()
				);
			}
		}
	}

	return 0;
}

void idl_context::setup_config()
{
	setup_config_log();
}

void idl_context::setup_config_log()
{
	const char* options[] = { "trace", "debug", "info" };

	for (int i = 0; i < 3; ++i)
	{
		if (config_.log_level == options[i])
		{
			wise::log()->set_level((spdlog::level::level_enum)i);
		}
	}

	if (wise::log()->level() == spdlog::level::debug ||
		wise::log()->level() == spdlog::level::trace)
	{
		WISE_INFO("Enabling parsing debugging output.");

		yydebug = 1;
	}
}
