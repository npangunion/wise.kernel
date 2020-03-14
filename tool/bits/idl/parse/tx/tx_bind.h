#pragma once 

#include "../idl_field.h"
#include <vector>

class tx_bind
{
public: 
	tx_bind()
	{
	}

	~tx_bind()
	{
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

	void set_return(const std::string& rt)
	{
		return_ = rt;
	}

	bool has_return() const
	{
		return !return_.empty();
	}

	const std::string& get_return() const
	{
		return return_;
	}

private:
	std::vector<idl_field*> fields_;
	std::string return_;
};