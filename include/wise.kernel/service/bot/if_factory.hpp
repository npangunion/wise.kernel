#pragma once 

#include "if_act.hpp"

#include <map>

namespace wise
{

class if_factory
{
public:
	using type_key = std::string;

public: 
	static if_factory& inst();

	void add(const if_::key& key, if_::checker checker);

	if_::checker get(const if_::key& key);

private: 
	if_factory() = default;
	if_factory(const if_factory&) = delete;

private:
	std::map<type_key, if_::checker> checkers_;
};

} // wise
