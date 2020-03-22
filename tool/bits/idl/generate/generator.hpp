#pragma once 

#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/core/result.hpp>
#include <wise.kernel/util/util.hpp>

#include <idl/parse/idl_expression.h>
#include <idl/parse/idl_field.h>
#include <idl/parse/idl_node_struct.h>
#include <iostream>

class idl_program;
class idl_symbol_table;

extern idl_program* g_program;
extern idl_symbol_table* g_symbols;

using result = wise::kernel::result<bool, std::string>;

class generator
{
public: 

public: 
	generator(idl_program* prog, std::ostream& os)
		: program_(prog)
		, os_(os)
	{
		WISE_ENSURE(program_);
		WISE_ENSURE(os_);
	}

	/// generate source code from prog into ostream
	virtual result generate() = 0;

protected: 
	bool is_enum(const idl_field* field) const;

	bool is_macro(const idl_field* field) const;

	bool is_serializable_field(const idl_field* field) const;

	std::string generator::get_namespace() const;

	std::string generator::get_topic_namespace() const;

	std::string generator::get_type_name(const idl_node* node) const;

	bool generator::is_short_type_field(const idl_field* field) const;

	void generator::inc_indent();

	void generator::dec_indent();

	std::ostream& generator::indent(std::ostream& os);

	void generator::reset_indent();

protected: 
	idl_program* program_ = nullptr;
	int indent_level_ = 0;
	std::ostream& os_;
};

