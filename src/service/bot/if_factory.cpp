#include "stdafx.h"
#include "if_factory.hpp"

namespace wise
{

if_factory& if_factory::inst()
{
	static if_factory inst_;

	return inst_;
}

void if_factory::add(const if_::key& key, if_::checker checker)
{
	checkers_[key] = checker;
}

if_::checker if_factory::get(const if_::key& key)
{
	auto iter = checkers_.find(key);
	if (iter == checkers_.end())
	{
		return if_::checker();
	}

	return iter->second;
}

} // wise
