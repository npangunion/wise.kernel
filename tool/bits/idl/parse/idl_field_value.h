#pragma once 

#include "idl_expression.h"

/**
 * field value
 */
class idl_field_value
{
public:
	idl_field_value()
	{
	}

	~idl_field_value()
	{
		delete default_expr_;
	}

	void set_default_expression(idl_expression* expr)
	{
		default_expr_ = expr;
	}

	const idl_expression* get_default_expression() const
	{
		return default_expr_;
	}

	void set_output()
	{
		output_ = true;
	}

	bool is_output() const
	{
		return output_;
	}

private: 
	idl_expression* default_expr_ = nullptr;
	bool output_ = false;
};