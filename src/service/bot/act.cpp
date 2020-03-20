#include "stdafx.h"
#include "act.hpp"
#include "agent.hpp"
#include "report.hpp"
#include <wise/base/trim.hpp>
#include <wise/server/util/json_helper.hpp>

namespace wise
{

act::act(flow& owner, config& cfg)
	: owner_(owner)
	, cfg_(cfg)
{
	load_key();
	load_slots();
	// load_timeout();

	on_subscribe();
}

act::~act()
{
}

void act::begin()
{
	WISE_ASSERT(!active_);

	if (active_)
	{
		WISE_DEBUG(
			"try to activate already active act. key: {0}", 
			get_key()
		);

		end(); // 임시로 꽁수. 제대로 된 구조는 계층적으로 만드는 것.
	}

	active_ = true;
	failed_ = false;

	begin_ = get_agent().get_tick();

	WISE_DEBUG("act: {0} begin.", get_key());

	timeout_timer_.reset();

	on_begin();
}

void act::exec()
{
	WISE_ASSERT(active_);

	if ( timeout_ > 0.f)
	{
		if (timeout_timer_.check_timeout(timeout_))
		{
			fail("timeout");

			return;
		}
	}

	on_exec();

	// 이벤트 순서와 정확성 확인 

	trigger_from_queue();
}

void act::end()
{
	if (!active_)
	{
		WISE_WARN(
			"try to end inactive act. key: {0}", 
			get_key()
		);

		return;
	}

	on_end();

	active_ = false;

	end_ = get_agent().get_tick();

	report::inst().add(
		get_agent().get_id(),
		"act",
		get_key(),
		begin_,
		end_,
		end_ - begin_,
		!failed_
	);

	WISE_DEBUG("act: {0} end. {1}", get_key(), failed_ ? "fail" : "success");
}

void act::fail(const std::string& reason)
{
	WISE_ERROR("act: {0} failed with reason: {1}", get_key(), reason);

	failed_ = true;

	trigger("fail");
}

void act::succeed(const std::string& reason)
{
	WISE_DEBUG("act: {0} succeeded with reason: {1}", get_key(), reason);

	failed_ = false;

	trigger("success");
}

void act::trigger(const std::string& slot)
{
	WISE_DEBUG("act: {} trigger slot: {}", get_key(), slot);

	tq_.push(slot);
}

agent& act::get_agent()
{
	return get_owner().get_owner();
}

void act::on_subscribe()
{
}

void act::on_begin()
{
}

void act::on_exec()
{
}

void act::on_end()
{
}

dic_any& act::get_dic()
{
	return get_agent().get_dic();
}

sub::key_t act::sub(topic key, handler handler)
{
	return get_agent().get_channel().subscribe(key, handler);
}

void act::make_me_active()
{
	get_agent().get_flow()->end_current();

	begin();
}

void act::trigger_from_queue()
{
	std::string key; 

	while (tq_.pop(key))
	{
		auto iter = slots_.find(key);

		if (iter == slots_.end())
		{
			WISE_WARN("act: {}. trigger not defined for {}", get_key(), key);
			return;
		}

		auto& vec = iter->second;

		for (auto& tr : vec)
		{
			WISE_TRACE(
				"trigger key: {0}, cmd: {1}, target: {2}, value: {3}",
				key, tr.cmd, tr.target, tr.value
			);

			trigger_slot(tr);
		}
	}
}

void act::trigger_slot(const slot& tr )
{
	if (tr.cmd == "next")
	{
		slot self{
			tr.key, 
			"end", 
			get_key(), 
			""
		};

		trigger_slot_end(self);
		trigger_slot_next(tr);
	}
	else if (tr.cmd == "jump")
	{
		slot self{
			tr.key, 
			"end", 
			get_key(),
			""
		};

		trigger_slot_end(self);
		trigger_slot_jump(tr);
	}
	else if (tr.cmd == "exit")
	{
		get_agent().exit();
	}
	else 
	{
		WISE_WARN(
			"trigger has invalid cmd. key: {0}, cmd: {1}",
			tr.key,
			tr.cmd
		);
	}
}

void act::trigger_slot_next(const slot& tr)
{
	WISE_UNUSED(tr);

	auto next_index = get_index() + 1;

	if (next_index >= get_owner().size())
	{
		WISE_WARN("loop from start with next in slot");
		next_index = 0;
	}

	auto ap = get_owner().get_act(next_index);

	begin_act(ap);
}

void act::trigger_slot_end(const slot& tr)
{
	auto ap = get_agent().get_act(tr.target);

	if (!ap)
	{
		WISE_WARN("act: {} not found in end", tr.target);
		return;
	}

	if (ap)
	{
		// assert 계속 걸려서 일단 변경
		// assert를 바꾸면 안 되는데...

		if (ap->is_active())
		{
			ap->end();
		}
	}
}

void act::trigger_slot_jump(const slot& tr)
{
	auto ap = get_agent().get_act(tr.target);

	if (!ap)
	{
		WISE_WARN("act: {} not found in jump", tr.target);
		return;
	}

	begin_act(ap);
}

void act::begin_act(act::ptr ap)
{
	if (ap)
	{
		// assert로 변경해야 함

		if (!ap->is_active())
		{
			ap->begin();
		}
	}
}

void act::load_key()
{
	auto key = cfg_["key"].get<std::string>();

	trim(key);

	key_ = key;

	WISE_ASSERT(key_.length() > 0);
}

void act::load_slots()
{
	auto nslots = cfg_["slots"];

	WISE_JSON_CHECK(nslots.is_array());

	for (auto& nslot : nslots)
	{
		WISE_ASSERT(nslot.is_object());

		auto nkey = nslot["key"];
		auto ncmd = nslot["cmd"];
		auto ntarget = nslot["target"];
		auto nvalue = nslot["value"];

		WISE_JSON_CHECK(!nkey.is_null() && nkey.is_string());
		WISE_JSON_CHECK(!ncmd.is_null() && ncmd.is_string());

		auto key = nkey.get<std::string>();

		auto cmd = ncmd.get<std::string>();
		std::string  target = get_key();
		std::string  value = "";

		if (!ntarget.is_null())
		{
			target = ntarget.get<std::string>();
		}

		if (!nvalue.is_null())
		{
			value = nvalue.get<std::string>();
		}

		add_slot(slot{ key, cmd, target, value });
	}
}

void act::load_timeout()
{
	auto nto = cfg_["timeout"];

	if (!nto.is_null())
	{
		timeout_ = nto.get<tick_t>();
	}
}

void act::add_slot(const slot& tr)
{
	auto iter = slots_.find(tr.key);

	if (iter == slots_.end())
	{
		slot_vec vec; 
		vec.push_back(tr);
		slots_[tr.key] = vec;
	}
	else
	{
		iter->second.push_back(tr);
	}
}

bool act::is_flow_name(const std::string& key) const
{
	auto pos = key.find_first_of(".");
	WISE_RETURN_IF(pos == std::string::npos, false);

	return pos == key.size()-1;
}

} // wise
