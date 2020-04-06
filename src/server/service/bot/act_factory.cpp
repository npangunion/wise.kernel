#include "stdafx.h"
#include "act_factory.hpp"

namespace wise
{

act_factory& act_factory::inst()
{
	static act_factory inst_;

	return inst_;
}

void act_factory::add(const type_key& key, act::creator creator)
{
	creators_[key] = creator;
}

act::ptr act_factory::create(flow& owner, act::config& cfg)
{
	auto type = cfg["type"];

	if (type.is_null() || !type.is_string())
	{
		return act::ptr();
	}

	auto tkey = type.get<type_key>();
	auto iter = creators_.find(tkey);

	if (iter == creators_.end())
	{
		WISE_ERROR(
			"act not found while creating. key: {0}", 
			type.get<type_key>()
		);

		return act::ptr();
	}

	return iter->second(owner, cfg);
}

} // wise
