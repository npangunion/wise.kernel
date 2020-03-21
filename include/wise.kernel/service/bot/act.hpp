#pragma once

#include <wise/service/bot/dic_any.hpp>
#include <wise/base/logger.hpp>
#include <wise/base/concurrent_queue.hpp>
#include <wise/base/tick.hpp>
#include <wise/base/json.hpp>
#include <wise/base/macros.hpp>
#include <wise/base/memory.hpp>
#include <wise/channel/channel.hpp>
#include <wise/server/util/types.hpp>
#include <wise/server/util/json_helper.hpp>
#include <cassert>
#include <string>
#include <memory>

namespace wise
{

class agent;
class flow;

/// 시작/성공/실패/종료의 이벤트와 jump/pump/end를 갖는 테스트 노드
class act : public std::enable_shared_from_this<act>
{
public: 
	using handler = sub::cb_t;
	using config = nlohmann::json;
	using ptr = std::shared_ptr<act>;
	using key = std::string;
	using creator = std::function<act::ptr(flow&, config&)>;

public: 
	act(flow& owner, config& cfg);

	virtual ~act();

	void begin();

	void exec();

	void end();

	void fail(const std::string& reason); 

	void succeed(const std::string& reason);

	void trigger(const std::string& slot);

	std::size_t set_index(std::size_t index)
	{
		index_ = index;
		return index_;
	}

	std::size_t get_index() const
	{
		return index_;
	}

	flow& get_owner()
	{
		return owner_;
	}

	const flow& get_owner() const
	{
		return owner_;
	}

	agent& get_agent();

	const std::string& get_key() const
	{
		return key_;
	}

	bool is_active() const
	{
		return active_;
	}

	bool equals(act::ptr other) const
	{
		return get_key() == other->get_key();
	}

protected:
	/// override to subscribe to topics. called from constructor
	virtual void on_subscribe();

	/// override to begin
	virtual void on_begin();

	/// override to execute 
	virtual void on_exec();

	/// override to end
	virtual void on_end();

	void make_me_active();

	/// get dic_any
	dic_any& get_dic();

	/// subscribe to agent channel w/ redirection from network
	sub::key_t sub(topic key, handler handler);

	void trigger_from_queue();

	config& get_config() 
	{
		return cfg_;
	}

private: 
	struct slot
	{
		std::string key;
		std::string cmd;
		std::string target;
		std::string value;
	};

	using slot_vec = std::vector<slot>;
	using slots = std::map<std::string, slot_vec>;
	using subs = std::vector<sub::key_t>;
	using trigger_queue = concurrent_queue<std::string>;

private: 
	void trigger_slot(const slot& tr);
	void trigger_slot_jump(const slot& tr);
	void trigger_slot_next(const slot& tr);
	void trigger_slot_end(const slot& tr);
	void begin_act(act::ptr ap);
	void load_key();
	void load_slots();
	void load_timeout();
	void add_slot(const slot& tr);
	bool is_flow_name(const std::string& key) const;

private:
	flow& owner_;
	key key_;
	config& cfg_;
	std::size_t index_;
	slots slots_;
	bool active_ = false;
	bool failed_ = false;
	tick_t begin_; 
	tick_t end_;
	trigger_queue tq_;
	simple_tick timeout_timer_;
	tick_t timeout_ = 0;
};

} // wise

/// 서비스 핸들러 얻는 편의 매크로
#define AGENT_GET_SERVICE_MONITOR() get_agent().get_server().get_service_monitor()

/// 서비스 링크 얻는 편의 매크로
#define AGENT_GET_SERVICE_REF(id) AGENT_GET_SERVICE_MONITOR()->get_ref(id)

#define ACT_SUBSCRIBE(evt) \
	sub(evt::get_topic(), WISE_CHANNEL_CB(on_##evt));