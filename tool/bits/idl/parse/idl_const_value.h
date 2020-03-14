#pragma once

#include "idl_type_enum.h"
#include <stdint.h>
#include <map>
#include <vector>
#include <string>

/**
* A const value is something parsed that could be a map, set, list, struct
* or whatever.
*
*/
class idl_const_value {
public:
	enum value_type { CV_INTEGER, CV_DOUBLE, CV_STRING, CV_IDENTIFIER, CV_UNKNOWN };

	idl_const_value() : intVal_(0), doubleVal_(0.0f), enum_((idl_type_enum*)0), valType_(CV_UNKNOWN) {}

	idl_const_value(int64_t val) : doubleVal_(0.0f), enum_((idl_type_enum*)0), valType_(CV_UNKNOWN) { set_integer(val); }

	idl_const_value(std::string val) : intVal_(0), doubleVal_(0.0f), enum_((idl_type_enum*)0), valType_(CV_UNKNOWN) { set_string(val); }

	void set_string(std::string val) 
	{
		valType_ = CV_STRING;
		stringVal_ = val;
	}

	std::string get_string() const { return stringVal_; }

	void set_integer(int64_t val) 
	{
		valType_ = CV_INTEGER;
		intVal_ = val;
	}

	int64_t get_integer() const 
	{
		return intVal_;
	}

	void set_double(double val) 
	{
		valType_ = CV_DOUBLE;
		doubleVal_ = val;
	}

	double get_double() const { return doubleVal_; }

	void set_identifier(std::string val) 
	{
		valType_ = CV_IDENTIFIER;
		identifierVal_ = val;
	}

	std::string get_identifier() const { return identifierVal_; }

	std::string get_identifier_name() const 
	{
		std::string ret = get_identifier();
		size_t s = ret.find('.');
		if (s == std::string::npos) {
			throw "error: identifier " + ret + " is unqualified!";
		}
		ret = ret.substr(s + 1);
		s = ret.find('.');
		if (s != std::string::npos) {
			ret = ret.substr(s + 1);
		}
		return ret;
	}

	std::string get_identifier_with_parent() const 
	{
		std::string ret = get_identifier();
		size_t s = ret.find('.');
		if (s == std::string::npos) {
			throw "error: identifier " + ret + " is unqualified!";
		}
		size_t s2 = ret.find('.', s + 1);
		if (s2 != std::string::npos) {
			ret = ret.substr(s + 1);
		}
		return ret;
	}

	void set_enum(idl_type_enum* tenum) { enum_ = tenum; }

	value_type get_type() const { if (valType_ == CV_UNKNOWN) { throw std::string("unknown idl_const_value"); } return valType_; }

private:
	std::string stringVal_;
	int64_t intVal_;
	double doubleVal_;
	std::string identifierVal_;
	idl_type_enum* enum_;
	value_type valType_;
};

