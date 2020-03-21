#pragma once

#include "idl_expression.h"
#include <wise.kernel/core/macros.hpp>

/** 
 * enum value
 */
class idl_enum_value  
{
public:
	idl_enum_value() = default;

	~idl_enum_value()
	{
		delete default_expr_;
	}

	void set_name(const std::string& name)
	{
		WISE_EXPECT(!name.empty());
		WISE_EXPECT(name_.empty());

		name_ = name;
	}

	void set_default_expression(idl_expression* expr)
	{
		WISE_EXPECT(expr);
		WISE_EXPECT(default_expr_ == nullptr);

		default_expr_ = expr;
	}

	const std::string& get_name() const
	{
		return name_;
	}

	const idl_expression* get_expr() const
	{
		return default_expr_;
	}

private: 
	std::string name_;
	idl_expression* default_expr_ = nullptr;
};
