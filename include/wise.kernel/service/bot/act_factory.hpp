#pragma once 

#include "act.hpp"

#include <map>

namespace wise
{

class act_factory
{
public:
	using type_key = std::string;

public: 
	static act_factory& inst();

	void add(const type_key& key, act::creator creator);

	act::ptr create(flow& owner, act::config& cfg);

private: 
	act_factory() = default;
	act_factory(const act_factory&) = delete;

private:
	std::map<type_key, act::creator> creators_;
};

} // wise

#define WISE_ADD_ACT(key, cls) \
	wise::act_factory::inst().add((key), \
	[](wise::flow& owner, wise::act::config& cfg)  \
	{ \
		return wise_shared<cls>(owner, cfg); \
	})