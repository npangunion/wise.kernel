#pragma once 

#include <vector>

#include "idl_node.h"
#include "idl_field.h"

class idl_node_struct : public idl_node
{
public:
	idl_node_struct()
	{
		type_ = Struct;
	}

	~idl_node_struct()
	{
		for (auto& fp : fields_)
		{
			delete fp;
		}
	}

	void add_field(idl_field* v) 
	{ 
		WISE_EXPECT(v);

		fields_.push_back(v); 
	}

	const std::vector<idl_field*>& get_fields() const 
	{ 
		return fields_; 
	}

private:
	std::vector<idl_field*> fields_;
};

