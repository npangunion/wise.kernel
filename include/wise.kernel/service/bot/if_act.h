#pragma once

#include "act.h"
#include <functional>

namespace wise
{

class if_ : public act
{
public: 
	using checker = std::function<bool (flow *)>;
	using key = std::string;

public: 
	if_(flow* owner, const config& cfg)
		: act(owner, cfg)
	{
		load();
	}

private: 
	virtual void on_begin() override
	{
		if (checker_)
		{
			if (checker_(get_owner()))
			{
				succeed("true!");
			}
			else
			{
				fail("false!");
			}
		}
		else
		{
			fail("checker pointer is invalid");
		}
	}

	void load();

private: 
	checker checker_;
};

} // wise
