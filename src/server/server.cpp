#include <pch.hpp>
#include <wise.kernel/server/server.hpp>
#include <wise.kernel/server/client_service.hpp>
#include <wise.kernel/server/actor_cluster.hpp>
#include <wise.kernel/server/cleanup_stack.hpp>
#include <wise.kernel/server/server_packets_factory.hpp>
#include <fstream>

namespace wise
{
namespace kernel
{

server::server()
	: cluster_(*this)
{
	WISE_ADD_ACTOR(client_service);

	add_server_packets();
}

server::~server()
{

}

server::result server::start(const std::string& file)
{
	config_file_ = file;

	// config
	{
		auto rc = load_config();
		WISE_RETURN_IF(!rc, result(false, error_code::fail_load_config));
	}

	return start();
}

server::result server::start()
{
	cleanup_stack<std::function<void()>> cs;

	// bits_node
	{
		bits_node_ = wise_unique<bits_node>(bits_config_);

		auto rc = bits_node_->start();
		WISE_RETURN_IF(!rc && cs.set_fail(), result(false, error_code::fail_start_bits_node));

		cs.push([this]() { bits_node_->finish(); });
	}

	// scheduler
	{
		auto rc = scheduler_.start(scheduler_cfg_);
		WISE_RETURN_IF(!rc && cs.set_fail(), result(false, error_code::fail_start_task_scheduler));

		cs.push([this]() { scheduler_.finish(); });
	}

	// cluster
	{
		bool rc = cluster_.start();
		WISE_RETURN_IF(!rc && cs.set_fail(), result(false, error_code::fail_start_actor_cluster));
	}

	return result(true, error_code::success);
}

server::result server::run()
{
	scheduler_.schedule();
	cluster_.run();

	return result(true, error_code::success);
}

void server::finish()
{
	scheduler_.finish();
	bits_node_->finish();
	cluster_.finish();

	actors_.cleanup();
}

server::result server::listen(const std::string& addr, channel::ptr ch)
{
	auto rc = bits_node_->listen(addr, ch);

	if (!rc)
	{
		return result(false, error_code::fail_listen);
	}

	return result(true, error_code::success);
}

server::result server::connect(const std::string& addr, channel::ptr ch)
{
	auto rc = bits_node_->connect(addr, ch);

	if (!rc)
	{
		return result(false, error_code::fail_connect);
	}

	return result(true, error_code::success);
}


bool server::load_config()
{
	std::ifstream fs(config_file_);

	if (!fs.is_open())
	{
		WISE_ERROR("cannot open configuration file: {}", config_file_);
		return false;
	}

	json_ = nlohmann::json::parse(fs);
	fs.close();

	try
	{
		auto rc = load_server_config();
		WISE_RETURN_IF(!rc, false);

		rc = load_cluster_config();
		WISE_RETURN_IF(!rc, false);

		rc = load_bits_config();
		WISE_RETURN_IF(!rc, false);

		rc = load_scheduler_config();
		WISE_RETURN_IF(!rc, false);

		rc = load_actors();
		WISE_RETURN_IF(!rc, false);
	}
	catch (nlohmann::json::exception& ex)
	{
		WISE_ERROR("exception: {}", ex.what());
		return false;
	}

	return true;
}

bool server::load_server_config()
{
	domain_ = json_["domain"].get<uint16_t>();
	id_generator_.setup(domain_);

	return true;
}

bool server::load_bits_config()
{
	if (json_["bits"].is_null())
	{
		return true; // use defaults
	}

	auto jbits = json_["bits"];
	auto juhc = jbits["use_hardware_concurrency"];

	if (!juhc.is_null())
	{
		bits_config_.use_hardware_concurreny = juhc.get<bool>();
	}

	if (!bits_config_.use_hardware_concurreny)
	{
		auto jcl = jbits["concurrency_level"];
		if (!jcl.is_null())
		{
			bits_config_.concurreny_level = jcl.get<int>();
		}
	}

	auto jedl = jbits["enable_detail_log"];
	if (!jedl.is_null())
	{
		bits_config_.enable_detail_log = jedl.get<bool>();
	}

	auto jstnd = jbits["set_tcp_no_delay"];
	if (!jstnd.is_null())
	{
		bits_config_.set_tcp_no_delay = jstnd.get<bool>();
	}

	auto jstl = jbits["set_tcp_linger"];
	if (!jstl.is_null())
	{
		bits_config_.set_tcp_linger = jstl.get<bool>();
	}

	auto jstra = jbits["set_tcp_reuse_addr"];
	if (!jstra.is_null())
	{
		bits_config_.set_tcp_reuse_addr = jstra.get<bool>();
	}

	auto jmax = jbits["max_session_count"];
	if (!jmax.is_null())
	{
		bits_config_.max_session_count = jmax.get<int>();
		WISE_ENSURE(bits_config_.max_session_count > 0);
	}

	return true;
}

bool server::load_cluster_config()
{
	auto jcluster = json_["cluster"];
	WISE_RETURN_IF(jcluster.is_null(), false);

	return cluster_.setup(jcluster);
}

bool server::load_scheduler_config()
{
	scheduler_cfg_.runner_count = json_["scheduler"]["runner_count"].get<int>();
	return true;
}

bool server::load_actors()
{
	WISE_THROW_IF(
		id_generator_.get_domain() == 0, 
		"domain must be set before loading actors"
	);

	auto jactors = json_.find("actors");

	for (auto& jactor : jactors)
	{
		auto aref = cluster_.create(jactor);
		WISE_RETURN_IF(!aref, false); // fail early
	}

	return true;
}

} // kernel
} // wise
