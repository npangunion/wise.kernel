#pragma once

#include "parse/idl_node.h"
#include <wise/base/result.hpp>
#include <string>
#include <unordered_map>

struct idl_symbol
{
	enum type
	{
		None,
		Enum,
		Struct,
		Message, 
		Tx
	};

	type type_;
	std::string full_name_;
	std::string simple_name_;
	idl_node* node;
};


class idl_symbol_table
{
public: 
	using result = wise::result<bool, idl_symbol>;

public:
	result add(const idl_symbol& sym);

	result lookup(const std::string& full_symbol);

	result lookup(const std::string& ns, const std::string& simple);

private: 
	using symbol_map = std::unordered_map<std::string, idl_symbol>;

	symbol_map symbols_;
};