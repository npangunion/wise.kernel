#include "stdafx.h"
#include "flow.hpp"
#include "agent.hpp"
#include "act_factory.hpp"
#include <wise/server/util/json_helper.hpp>

namespace wise
{

flow::flow(agent& owner, const std::string& key, config& cfg)
	: owner_(owner)
	, key_(key)
	, cfg_(cfg)
	, active_(false)
{
	load(cfg_);
}

flow::~flow()
{
}

bool flow::start()
{
	WISE_ASSERT(!active_);

	active_ = true;

	if (act_0_)
	{
		act_0_->begin();
	}

	WISE_INFO("start flow: {0}", get_key());

	return true;
}

void flow::exec()
{
	WISE_ASSERT(active_);

	for ( auto& ap: acts_ )
	{ 
		if (ap->is_active())
		{
			ap->exec();
		}
	}
}

void flow::stop()
{
	WISE_ASSERT(active_);

	for ( auto&  ap : acts_ )
	{ 
		if (ap->is_active())
		{
			ap->end();
		}
	}

	acts_.clear();

	active_ = false;

	WISE_INFO("stop flow: {0}", get_key());
}

act::ptr flow::find(const act::key& key)
{
	return get_act(key);
}

void flow::end_current()
{
	for ( auto&  ap : acts_ )
	{ 
		if (ap->is_active())
		{
			ap->end();
		}
	}
}

void flow::load(const config& nacts)
{
	WISE_ASSERT(nacts.is_array());

	for (int i = 0; i<nacts.size(); ++i)
	{
		auto nact = nacts[i];
		WISE_JSON_CHECK(!nact.is_null() && nact.is_object());

		auto ntype = nact["type"];
		WISE_JSON_CHECK(!ntype.is_null() && ntype.is_string());

		auto ap = act_factory::inst().create(
				*this, 
				nact	
			);

		if (ap)
		{
			load_act_push(ap);

			if (!act_0_)
			{
				act_0_ = ap;
			}
		}
		else
		{
			WISE_THROW_FMT("failed to create {}", nact.dump());
		}
	}
}

void flow::load_act_push(act::ptr ap)
{
	// reset or add
	acts_.push_back(ap);

	auto index = acts_.size() - 1;
	ap->set_index(index);
	key_index_.set(ap->get_key(), ap->get_index());
}

} // wise
