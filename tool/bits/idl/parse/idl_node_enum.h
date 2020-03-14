#pragma once 

#include <vector>

#include "idl_node.h"
#include "idl_enum_value.h"

class idl_node_enum : public idl_node
{
public:
	idl_node_enum()
		: idl_node()
	{
		type_ = Enum;
	}

	~idl_node_enum()
	{
		for (auto& vp : values_)
		{
			delete vp;
		}

		values_.clear();
	}

	void add_value(idl_enum_value* v) 
	{ 
		WISE_EXPECT(v);

		values_.push_back(v); 
	}

	const std::vector<idl_enum_value*>& get_values() const 
	{ 
		return values_;  
	}

private:
	std::vector<idl_enum_value*> values_;
};

