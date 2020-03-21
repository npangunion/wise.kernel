#pragma once 

#include <wise/server/handler/handler.hpp>
#include <wise/server/service/service.hpp>
#include <wise/service/bot/agent.hpp>
#include <wise/service/bot/act_factory.hpp>

namespace wise
{

class bot_service : public service
{
public:
	bot_service(server& _server, const std::string& name, const config& _config);

	~bot_service();

protected: 
	/// post to agent by session auth id
	void post_by_session(packet::ptr pkt);

	/// post to agent by tx eid
	void post_by_tx(packet::ptr pkt);

	/// post to agent by id (index in array) 
	void post_by_agent_id(uint64_t id, packet::ptr pkt);

	/// get index from agnet id
	uint32_t get_index_from_agent_id(uint64_t id);

	/// check valid index
	bool is_valid_agent_index(uint32_t index);

private:
	using vec_post = std::vector<topic>;

private:
	bool on_start() override;

	result on_execute() override;

	void on_finish() override;

	void on_sys_connect_failed(message::ptr mp);

	void on_sys_session_ready(message::ptr mp);

	void on_sys_session_closed(message::ptr mp);

	void on_tx_message(message::ptr mp);

	void on_session_message(message::ptr mp);

	bool check_exit();

	bool load_config();

	bool load_test_config();

	bool load_post_config();

	void subscribe_posts();

	bool start_report();

	bool start_agents();

	void stop_agents();

	void add_internal_acts();

private:
	simple_tick tick_log_;
	config test_config_;
	uint32_t begin_index_ = 1;
	uint32_t agent_count_ = 1;
	std::vector<agent::ptr> agents_;
	vec_post tx_posts_;
	vec_post ss_posts_;
};

}  // wise
