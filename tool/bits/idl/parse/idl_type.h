#pragma once 

#include <wise/base/macros.hpp>
#include <string>

class idl_type abstract
{
public:
	enum type
	{
		simple, 
		full, 
		vec, 
		map, 
		topic, 
		macro, 
		option
	};

	idl_type() = default;

	virtual ~idl_type() 
	{
	}

	void set_name(const std::string& name) 
	{ 
		WISE_EXPECT(!name.empty());

		name_ = name; 
	}

	const std::string& get_name() const 
	{ 
		return name_; 
	}

	bool is_simple() const
	{
		return type_ == type::simple;
	}

	type get_type() const
	{
		return type_;
	}

protected:
	std::string name_ = "unknown";
	type type_ = type::simple;
};
