#pragma once

#include "idl_type_container.h"
#include <cstdlib>
#include <sstream>

class idl_type_topic : public idl_type
{
public:
	idl_type_topic()
	{
		type_ = type::topic;
	}
	
	~idl_type_topic()
	{
	}

	void set_identifier(const char* id)
	{
		identifier_ = id;

		std::ostringstream oss;

		oss << "topic: " << id;

		set_name(oss.str());
	}

	const std::string& get_identifier() const
	{
		return identifier_;
	}


private:
	std::string identifier_;
};
