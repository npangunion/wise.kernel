#include "stdafx.h"

#include "if_act.hpp"
#include "agent.hpp"
#include "if_factory.hpp"

namespace wise
{

void if_::load()
{
	auto key = get_config()["checker"].get<std::string>();

	checker_ = if_factory::inst().get(key);

	if (!checker_)
	{
		WISE_WARN("checker not found.key: {0}", key);
	}
}

} // wise
