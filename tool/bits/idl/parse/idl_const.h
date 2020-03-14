#pragma once 

#include "idl_type.h"
#include "idl_const_value.h"

/**
* A const is a constant value defined across languages that has a type and
* a value. The trick here is that the declared type might not match the type
* of the value object, since that is not determined until after parsing the
* whole thing out.
*
*/
class idl_const 
{
public:
	idl_const(idl_type* type, std::string name, idl_const_value* value)
		: type_(type), name_(name), value_(value) {}

	idl_type* get_type() const { return type_; }

	std::string get_name() const { return name_; }

	idl_const_value* get_value() const { return value_; }

private:
	idl_type* type_;
	std::string name_;
	idl_const_value* value_;
};