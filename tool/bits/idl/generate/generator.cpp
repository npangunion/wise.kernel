#include <pch.hpp>
#include "generator.hpp"
#include <idl/idl_symbol_table.hpp>
#include <idl/parse/idl_program.h>
#include <idl/parse/idl_node.h>
#include <idl/parse/idl_node_include.h>
#include <idl/parse/idl_node_enum.h>
#include <idl/parse/idl_node_struct.h>
#include <idl/parse/idl_node_message.h>
#include <idl/parse/idl_node_namespace.h>
#include <idl/parse/idl_type_full.h>
#include <idl/parse/idl_type_macro.h>
#include <idl/parse/idl_type_map.h>
#include <idl/parse/idl_type_simple.h>
#include <idl/parse/idl_type_topic.h>
#include <idl/parse/idl_type_vec.h>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/macros.hpp>
#include <wise.kernel/core/util.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

bool generator::is_enum(const idl_field* field) const
{
	auto ftype = field->get_type();
	const auto& fn = ftype->get_name();

	// try with field type name. ex, hello::type1
	{
		auto rc = g_symbols->lookup(fn);

		if (rc && rc.value.type_ == idl_symbol::Enum)
		{
			return true;
		}
	}

	auto ffn = g_program->get_fullname(fn);

	// try with namespace attached. ex, program::hello::type1
	{
		auto rc = g_symbols->lookup(ffn);

		if (rc && rc.value.type_ == idl_symbol::Enum)
		{
			return true;
		}
	}

	return false;
}

bool generator::is_macro(const idl_field* field) const
{
	auto ftype = field->get_type();

	return ftype->get_type() == idl_type::macro;
}

bool generator::is_serializable_field(const idl_field* field) const
{
	WISE_ASSERT(field);

	auto ttype = field->get_type();
	WISE_ASSERT(ttype);

	return	ttype->get_type() != idl_type::macro &&
			ttype->get_type() != idl_type::topic && 
			ttype->get_type() != idl_type::option;
}

std::string generator::get_namespace() const
{
	std::ostringstream oss;

	auto ns = program_->get_namespace();

	oss << "::";

	if (ns)
	{
		auto ns_node = static_cast<const idl_node_namespace*>(ns);
		auto lst = ns_node->get_namespace();

		for (size_t i = 0; i < lst.size(); ++i)
		{
			oss << lst[i] << "::";
		}
	}
	else
	{
		oss << "wise::kernel::";
	}

	return oss.str();
}

std::string generator::get_topic_namespace() const
{
	return "topics";
}

std::string generator::get_type_name(const idl_node* node) const
{
	switch (node->get_type())
	{
	case idl_node::Enum:
		return "enum";
	case idl_node::Message:
		return "message";
	case idl_node::Include:
		return "include";
	case idl_node::Namespace:
		return "namespace";
	case idl_node::Struct:
		return "struct";
	case idl_node::Tx:
		return "tx";
	default: 
		return "Unknown";
	}
}

bool generator::is_short_type_field(const idl_field* field) const
{
	auto ftype = field->get_type();
	bool is_short_type = false;

	if (ftype->get_type() == idl_type::simple)
	{
		auto stype = static_cast<const idl_type_simple*>(ftype);
		auto stt = stype->get_simple_type();
		is_short_type = (stt == idl_type_simple::TYPE_I16 || stt == idl_type_simple::TYPE_U16);
	}

	return is_short_type;
}

void generator::inc_indent()
{
	WISE_ASSERT(indent_level_ >= 0);
	indent_level_++;
}

void generator::dec_indent()
{
	WISE_ASSERT(indent_level_ >= 0);

	indent_level_ = std::max(0, indent_level_ - 1);
}


std::ostream& generator::indent(std::ostream& os)
{
	WISE_ASSERT(indent_level_ >= 0);

	for (int i = 0; i < indent_level_; ++i)
	{
		os << "\t";
	}

	return os;
}

void generator::reset_indent()
{
	indent_level_ = 0;
}
