#pragma once

#include <cstdlib>
#include "idl_type.h"

class idl_type_simple : public idl_type 
{
public:
	/**
	* Enumeration of thrift type types
	*/
	enum class types {
		TYPE_STRING,
		TYPE_USTRING,
		TYPE_BOOL,
		TYPE_I8,
		TYPE_I16,
		TYPE_I32,
		TYPE_I64,
		TYPE_U8,
		TYPE_U16,
		TYPE_U32,
		TYPE_U64,
		TYPE_FLOAT,
		TYPE_DOUBLE, 
		TYPE_TIMESTAMP
	};

	idl_type_simple(types type)
		: idl_type()
		, simple_type_(type)
	{
		idl_type::type_ = type::simple;

		set_name(type_name(simple_type_));
	}

	types get_simple_type() const 
	{ 
		return simple_type_; 
	}

	bool is_string() const 
	{ 
		return simple_type_ == types::TYPE_STRING; 
	}

	bool is_bool() const 
	{ 
		return simple_type_ == types::TYPE_BOOL; 
	}

	static std::string type_name(types tbase) 
	{
		switch (tbase) {
		case types::TYPE_STRING:
			return "string";
		case types::TYPE_USTRING:
			return "ustring";
		case types::TYPE_BOOL:
			return "bool";
		case types::TYPE_I8:
			return "i8";
		case types::TYPE_I16:
			return "i16";
		case types::TYPE_I32:
			return "i32";
		case types::TYPE_I64:
			return "i64";
		case types::TYPE_U8:
			return "u8";
		case types::TYPE_U16:
			return "u16";
		case types::TYPE_U32:
			return "u32";
		case types::TYPE_U64:
			return "u64";
		case types::TYPE_FLOAT:
			return "float";
		case types::TYPE_DOUBLE:
			return "double";
		case types::TYPE_TIMESTAMP:
			return "timestamp";
		default:
			return "(unknown)";
		}
	}

	std::string get_typename_in(const std::string& lang) const
	{
		if (lang == "c#")
		{
			return get_typename_in_csharp(simple_type_);
		}
		else if (lang == "c++")
		{
			return get_typename_in_cplus(simple_type_);
		}
		else if (lang == "sql")
		{
			return get_typename_in_sql(simple_type_);
		}

		return get_name();
	}

	std::string get_typename_in_csharp(types type) const
	{
		switch (type) 
		{
		case types::TYPE_STRING:
			return "string";
		case types::TYPE_USTRING:
			return "string";
		case types::TYPE_BOOL:
			return "bool";
		case types::TYPE_I8:
			return "sbyte";
		case types::TYPE_I16:
			return "short";
		case types::TYPE_I32:
			return "int";
		case types::TYPE_I64:
			return "long";
		case types::TYPE_U8:
			return "byte";
		case types::TYPE_U16:
			return "ushort";
		case types::TYPE_U32:
			return "uint";
		case types::TYPE_U64:
			return "ulong";
		case types::TYPE_FLOAT:
			return "float";
		case types::TYPE_DOUBLE:
			return "double";
		case types::TYPE_TIMESTAMP:
			return "wise.timestamp";
		default:
			return "(unknown)";
		}
	}

	std::string get_typename_in_cplus(types type) const
	{
		switch (type) 
		{
		case types::TYPE_STRING:
			return "std::string";
		case types::TYPE_USTRING:
			return "std::wstring";
		case types::TYPE_BOOL:
			return "bool";
		case types::TYPE_I8:
			return "int8_t";
		case types::TYPE_I16:
			return "int16_t";
		case types::TYPE_I32:
			return "int32_t";
		case types::TYPE_I64:
			return "int64_t";
		case types::TYPE_U8:
			return "uint8_t";
		case types::TYPE_U16:
			return "uint16_t";
		case types::TYPE_U32:
			return "uint32_t";
		case types::TYPE_U64:
			return "uint64_t";
		case types::TYPE_FLOAT:
			return "float";
		case types::TYPE_DOUBLE:
			return "double";
		case types::TYPE_TIMESTAMP:
			return "wise::kernel::timestamp";
		default:
			return "(unknown)";
		}
	}


	std::string get_typename_in_sql(types type) const
	{
		switch (type)
		{
		case types::TYPE_STRING:
			return "VARCHAR";
		case types::TYPE_USTRING:
			return "NVARCHAR";
		case types::TYPE_BOOL:
			return "BIT";
		case types::TYPE_I8:
			return "TINYINT";
		case types::TYPE_I16:
			return "SMALLINT";
		case types::TYPE_I32:
			return "INT";
		case types::TYPE_I64:
			return "BIGINT";
		case types::TYPE_U8:
			return "TINYINT";
		case types::TYPE_U16:
			return "SMALLINT";
		case types::TYPE_U32:
			return "INT";
		case types::TYPE_U64:
			return "BIGINT";
		case types::TYPE_FLOAT:
			return "FLOAT";
		case types::TYPE_DOUBLE:
			return "DOUBLE";
		case types::TYPE_TIMESTAMP:
			return "TIMESTAMP";
		default:
			return "(unknown)";
		}
	}

private:
	types simple_type_;
};
