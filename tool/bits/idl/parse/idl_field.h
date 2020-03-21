#pragma once 

#include "idl_type.h"
#include "idl_expression.h"
#include "idl_field_value.h"
#include <wise.kernel/core/macros.hpp>
#include <sstream>

/**
 * struct / message field  
 */
class idl_field
{
public:
	idl_field() = default;

	~idl_field()
	{
		delete type_;
		delete value_;
	}

	void set_type(idl_type* type) 
	{ 
		WISE_EXPECT(type);
		WISE_EXPECT(!type_);

		type_ = type;  
	}

	void set_variable_name(const std::string& var_name) 
	{ 
		WISE_EXPECT(!var_name.empty());
		WISE_EXPECT(var_name_.empty());

		var_name_ = var_name;  
	}

	void set_field_value(idl_field_value* fv) 
	{ 
		WISE_EXPECT(fv);
		WISE_EXPECT(!value_);

		value_ = fv;  
	}

	const idl_type* get_type() const
	{
		return type_;
	}

	const std::string& get_variable_name() const
	{
		return var_name_;
	}

	const idl_field_value* get_field_value() const
	{
		return value_;
	}

	const char* get_namespace_separator(const std::string& lang) const
	{
		if (lang == "c#")
		{
			return ".";
		}

		return "::";
	}

	/// Hack!!! 생성시에만 한 곳에서 사용. 
	void forget_type()
	{
		type_ = nullptr;
	}

private: 
	idl_type* type_ = nullptr;
	std::string var_name_;
	idl_field_value* value_ = nullptr;
};