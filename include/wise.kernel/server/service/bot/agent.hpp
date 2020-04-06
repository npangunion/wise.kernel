#pragma once

#include "flow.hpp"
#include "dic_any.hpp"
#include <wise/base/concurrent_queue.hpp>
#include <wise/net/session.hpp>
#include <wise/server/handler/subscription_handler.hpp>
#include <wise/server/service/service.hpp>
#include <wise/base/tick.hpp>
#include <wise/base/memory.hpp>
#include <random>

namespace wise
{

class agent : public subscription_handler
{
public:
	using config = nlohmann::json;
	using ptr = std::shared_ptr<agent>;

public:
	agent(server& _server, uint32_t index, config& cfg);

	~agent();

	void exit()
	{
		if (get_state() != state::finished)
		{
			task::cancel(); // finish
		}
	}

	session_ref get_session(const std::string& name);

	bool  send(const std::string& name, packet::ptr mp);

	service_ref::ptr get_service_ref(service_id_t id) const;

	unsigned int get_index() const
	{
		return index_;
	}

	const std::string& get_id() const
	{
		return id_;
	}

	const std::string& get_pw() const
	{
		return pw_;
	}

	flow::ptr get_flow() const
	{
		return flow_;
	}

	dic_any& get_dic()
	{
		return dic_;
	}

	const dic_any& get_dic() const 
	{
		return dic_;
	}

	act::ptr get_act(const act::key& key) const;

	bool has_act(const act::key& key) const;

	std::size_t get_random();

private: 
	bool on_start() override;

	result on_execute() override;

	void on_finish() override;

private: 
	void load();
	void load_id();
	void load_pw();
	void load_flow();

private:
	uint32_t index_ = 0;
	config cfg_;
	std::string id_;
	std::string pw_;
	flow::ptr flow_;
	bool active_ = false;
	bool exit_ = false;
	dic_any dic_;
	int exit_stage_ = 0;
	float begin_ = 0;
	float end_ = 0;
	std::mt19937 mt_rand_;
	fine_tick tick_;
};

} // wise
