#pragma once 

#include "../idl_field.h"
#include <vector>

class tx_result_set
{
public:
	tx_result_set()
	{
	}

	~tx_result_set()
	{
	}

	void set_single()
	{
		single_ = true;
	}

	void set_multi() 
	{
		single_ = false;
	}

	bool is_single() const
	{
		return single_;
	}

	bool is_multi() const
	{
		return !single_;
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
	bool single_ = true;
	std::vector<idl_field*> fields_;
};