#pragma once

#include <cstdlib>
#include "idl_type_container.h"

#include <sstream>

class idl_type_map : public idl_type_container 
{
public:
	idl_type_map()
	{
		type_ = type::map;
	}
	
	~idl_type_map()
	{
		delete key_type_;
		delete value_type_;
	}

	void set_key_type(idl_type* vtype)
	{
		WISE_ASSERT(vtype);

		key_type_ = vtype;
	}

	const idl_type* get_key_type() const
	{
		return key_type_;
	}

	void set_value_type(idl_type* vtype)
	{
		WISE_ASSERT(vtype);

		value_type_ = vtype;


		std::ostringstream oss;

		oss << "map<";
		if (key_type_)
		{
			oss << key_type_->get_name();
			oss << ", ";
		}

		if (value_type_)
		{
			oss << value_type_->get_name();
		}
		oss << ">";

		set_name(oss.str());
	}

	const idl_type* get_value_type() const
	{
		return value_type_;
	}

private:
	idl_type* key_type_ = nullptr;
	idl_type* value_type_ = nullptr;
};
