#include <pch.hpp>
#include "csharp_generator.hpp"
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
#include <wise.kernel/util/util.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

result csharp_generator::generate()
{
	auto rc = generate_nodes();

	if (rc && program_->has_message())
	{
		rc = generate_factory();
	}

	return rc;
}

result csharp_generator::generate_nodes()
{
	WISE_EXPECT(program_);

	WISE_INFO("Generating file: {}", program_->get_path());

	// prolog
	{
		auto rc = generate_prolog();
		WISE_RETURN_IF(!rc, rc);
	}

	// include
	{
		bool gen = false;
		auto& nodes = program_->get_nodes();

		for (auto& node : nodes)
		{
			if (node->get_type() == idl_node::Type::Include)
			{
				auto rc = generate_include(node);
				WISE_RETURN_IF(!rc, rc);

				gen = true;
			}
		}

		if (gen)
		{
			os_ << std::endl;
		}
	}

	// namespace begin
	{
		auto rc = generate_namespace_begin(os_);
		WISE_RETURN_IF(!rc, rc);
	}

	auto& nodes = program_->get_nodes();

	for (auto& node : nodes)
	{
		WISE_INFO("Generating node: {}/{}", get_type_name(node), node->get_name());

		switch (node->get_type())
		{
		case idl_node::Type::Enum:
			{
				auto rc = generate_enum(node);
				WISE_RETURN_IF(!rc, rc);
			}
			break; 
			case idl_node::Type::Struct: 
			{
				auto rc = generate_struct(node);
				WISE_RETURN_IF(!rc, rc);
			}
			break; 
			case idl_node::Type::Message:
			{
				auto rc = generate_message(node);
				WISE_RETURN_IF(!rc, rc);
			}
			break;
			case idl_node::Type::Tx:
			{
				WISE_INFO("Tx is not supported for C#");
				break;
			}
			break;
			case idl_node::Type::Include:
			{
				// ignore
			}
			break;
			case idl_node::Type::Namespace:
			{
				// ignore
			}
			break;

			default: 
			{
				// not implemented yet.
				return result(false, "Not implemented node type");
			}
			break;
		}
	}

	// namespace end
	{
		auto rc = generate_namespace_end(os_);
		WISE_RETURN_IF(!rc, rc);
	}

	os_ << std::endl;

	// serialize

	reset_indent();

	// epilog 
	{
		auto rc = generate_epilog();
		WISE_RETURN_IF(!rc, rc);
	}

	WISE_INFO("Completed {}", program_->get_path());

	return result(true, "Success");
}

result csharp_generator::generate_factory()
{
	WISE_EXPECT(program_);

	//  _factory.cpp 파일에 추가
	// 

	namespace fs = boost::filesystem;

	std::string ext_removed = wise::kernel::remove_file_extension(program_->get_path());

	fs::path er_file(ext_removed);

	std::string filename = wise::kernel::get_filename(ext_removed);

	// cs file
	{
		auto hfile = ext_removed;
		hfile.append("_factory.cs");

		std::ofstream os(hfile.c_str());

		fs::path inc(program_->get_path());
		inc.replace_extension(".hpp");

		os << "using System;" << std::endl;
		os << "using wise;" << std::endl;
		os << std::endl;
		os << std::endl;

		os << "namespace ";

		os << "void add_" << filename << "()" << std::endl; 
		os << "{" << std::endl;

		auto& nodes = program_->get_nodes();

		inc_indent();

		for (auto& node : nodes)
		{
			if (node->get_type() == idl_node::Type::Message)
			{

			}
		}

		dec_indent();

		os << "}" << std::endl;
	}

	return result(true, "Success");
}

result csharp_generator::generate_include(const idl_node* node)
{
	WISE_UNUSED(node);

	return result(true, "Success");
}

result csharp_generator::generate_enum(const idl_node* node)
{
	auto en = static_cast<const idl_node_enum*>(node);

	os_ << std::endl;
	os_ << "public enum " << en->get_name() << std::endl;
	os_ << "{" << std::endl;

	auto vals = en->get_values();

	inc_indent();

	for (auto& v : vals)
	{
		indent(os_) << v->get_name(); 

		auto dv = v->get_expr();

		if (dv)
		{
			os_ << " = "; 

			generate_expression(nullptr, dv);
		}

		os_ << "," << std::endl;
	}

	dec_indent();

	os_ << "}" << std::endl;
	os_ << std::endl;


	return result(true, "Success");
}

result csharp_generator::generate_struct(const idl_node* node)
{
	auto sn = static_cast<const idl_node_struct*>(node);

	os_ << std::endl;
	os_ << "public class " << sn->get_name() << " : IPackable " << std::endl;
	os_ << "{" << std::endl;

	// field 생성 
	auto fields = sn->get_fields();

	for (auto& field : fields)
	{
		auto rc = generate_field(field);

		WISE_RETURN_IF(!rc, rc);
	}

	os_ << std::endl;

	// serialize
	{
		// pack
		{
			auto rc = generate_struct_pack(node);
			WISE_RETURN_IF(!rc, rc);
		}

		os_ << std::endl;

		// unpack
		{
			auto rc = generate_struct_unpack(node);
			WISE_RETURN_IF(!rc, rc);
		}
	};

	os_ << "}" << std::endl;
	os_ << std::endl;

	return result(true, "Success");
}


result csharp_generator::generate_struct_pack(const idl_node* node)
{
	auto sn = static_cast<const idl_node_struct*>(node);
	WISE_ASSERT(sn);

	inc_indent();

	indent(os_) << "public bool Pack(Packer packer, Stream stream) " << std::endl;
	indent(os_) << "{" << std::endl;

	// fields
	{
		inc_indent();

		auto fields = sn->get_fields();

		for (auto& field : fields)
		{
			if (is_serializable_field(field))
			{
				if (is_enum(field))
				{
					indent(os_) << "packer.PackEnum(stream, "
						<< field->get_variable_name() << ");"
						<< std::endl;
				}
				else
				{
					if (is_short_type_field(field))
					{
						indent(os_) << "packer.PackShort(stream, " << field->get_variable_name() << ");" << std::endl;
					}
					else
					{
						indent(os_) << "packer.Pack(stream, " << field->get_variable_name() << ");" << std::endl;
					}
				}
			}
			else
			{
				if (is_macro(field))
				{
					generate_field_macro(os_, field);
				}
			}
		}

		indent(os_) << "return true;" << std::endl;

		dec_indent();
	}

	indent(os_) << "}" << std::endl;

	dec_indent();

	return result(true, "Success");
}

result csharp_generator::generate_struct_unpack(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	inc_indent();

	indent(os_) << "public bool Unpack(Packer packer, Stream stream) " << std::endl;
	indent(os_) << "{" << std::endl;

	// fields
	{
		inc_indent();

		auto fields = sn->get_fields();

		for (auto& field : fields)
		{
			if (is_serializable_field(field))
			{
				if (is_enum(field))
				{
					indent(os_) << "packer.UnpackEnum(stream, "
						<< " out " 
						<< field->get_variable_name() << ");"
						<< std::endl;
				}
				else
				{
					if (is_short_type_field(field))
					{
						indent(os_) << "packer.UnpackShort(stream, ";
					}
					else
					{
						indent(os_) << "packer.Unpack(stream, ";
					}

					if (field->get_type()->get_type() != idl_type::type::vec)
					{
						os_ << " out ";
					}

					os_ << field->get_variable_name() << ");" << std::endl;
				}
			}
			else
			{
				if (is_macro(field))
				{
					generate_field_macro(os_, field);
				}
			}
		}

		indent(os_) << "return true;" << std::endl;

		dec_indent();
	}

	indent(os_) << "}" << std::endl;

	dec_indent();

	return result(true, "Success");
}

result csharp_generator::generate_message(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	os_ << std::endl;

	std::string super_class = "Packet";

	auto sc = sn->get_super_class();

	if (sc)
	{
		auto scft = static_cast<const idl_type_full*>(sc);
		super_class = scft->get_typename_in("c#");

		if (super_class == "wise.hx")
		{
			return result(true, "Skip hx genration");
		}
	}

	indent(os_) << "public class " << sn->get_name() << " : " << super_class << std::endl;
	indent(os_) << "{" << std::endl;

	// field 생성 
	{
		auto fields = sn->get_fields();

		for (auto& field : fields)
		{
			auto rc = generate_field(field);

			WISE_RETURN_IF(!rc, rc);
		}
	}

	os_ << std::endl;
	os_ << std::endl;

	// topic 
	{
		auto rc = generate_message_topic(node);

		if (!rc)
		{
			WISE_ERROR("Fail to generate topic: {}", rc.value);
		}

		WISE_RETURN_IF(!rc, result(false, "Failed to generate topic"));
	}

	os_ << std::endl;

	inc_indent();

	// constructor
	{
		indent(os_) << "public " << node->get_name() << "()" << std::endl;
		indent(os_) << ": base(GetTopic())" << std::endl;
		indent(os_) << "{" << std::endl;

		if (sn->is_set_enable_cipher())
		{
			indent(os_) << "\tIsEnableCipher = true;" << std::endl;
		}
		if (sn->is_set_enable_checksum())
		{
			indent(os_) << "\tIsEnableChecksum = true;" << std::endl;
		}
		if (sn->is_set_enable_sequence())
		{
			indent(os_) << "\tIsEnableSequence = true;" << std::endl;
		}

		indent(os_) << "}" << std::endl;
	}

	dec_indent();

	os_ << std::endl;

	// serialize
	{
		// pack
		{
			auto rc = generate_message_pack(node);
			WISE_RETURN_IF(!rc, rc);
		}

		os_ << std::endl;

		// unpack
		{
			auto rc = generate_message_unpack(node);
			WISE_RETURN_IF(!rc, rc);
		}
	}

	indent(os_) << "}" << std::endl;
	os_ << std::endl;

	return result(true, "Success");
}

result csharp_generator::generate_message_topic(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	auto fields = sn->get_fields();

	for (auto& field : fields)
	{
		auto ttype = field->get_type();

		if (ttype->get_type() == idl_type::type::topic)
		{
			const auto topic_type = static_cast<const idl_type_topic*>(ttype);
			auto id = topic_type->get_identifier();

			return generate_topic(topic_type, id);
		}
	}

	return result(false, "Topic field not found");
}

result csharp_generator::generate_message_pack(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	inc_indent();

	indent(os_) << "public override bool Pack(Packer packer, Stream stream) " << std::endl;
	indent(os_) << "{" << std::endl;

	// fields
	{
		inc_indent();

		auto fields = sn->get_fields();

		for (auto& field : fields)
		{
			if (is_serializable_field(field))
			{
				if (is_enum(field))
				{
					indent(os_) << "packer.PackEnum(stream, "
						<< field->get_variable_name() << ");"
						<< std::endl;
				}
				else
				{
					if (is_short_type_field(field))
					{
						indent(os_) << "packer.PackShort(stream, " << field->get_variable_name() << ");" << std::endl;
					}
					else
					{
						indent(os_) << "packer.Pack(stream, " << field->get_variable_name() << ");" << std::endl;
					}
				}
			}
			else
			{
				if (is_macro(field))
				{
					generate_field_macro(os_, field);
				}
			}
		}

		indent(os_) << "return true;" << std::endl;

		dec_indent();
	}

	indent(os_) << "}" << std::endl;

	dec_indent();

	return result(true, "Success");
}

result csharp_generator::generate_message_unpack(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	inc_indent();

	indent(os_) << "public override bool Unpack(Packer packer, Stream stream) " << std::endl;
	indent(os_) << "{" << std::endl;

	// fields
	{
		inc_indent();

		auto fields = sn->get_fields();

		for (auto& field : fields)
		{
			if (is_serializable_field(field))
			{
				if (is_enum(field))
				{
					indent(os_) << "packer.UnpackEnum(stream, "
						<< "out "
						<< field->get_variable_name() << ");"
						<< std::endl;
				}
				else
				{
					if (is_short_type_field(field))
					{
						indent(os_) << "packer.UnpackShort(stream, ";
					}
					else
					{
						indent(os_) << "packer.Unpack(stream, ";
					}

					if (field->get_type()->get_type() != idl_type::vec)
					{
						os_ << "out ";
					}

					os_ << field->get_variable_name() << ");" << std::endl;
				}
			}
			else
			{
				if (is_macro(field))
				{
					generate_field_macro(os_, field);
				}
			}
		}

		indent(os_) << "return true;" << std::endl;

		dec_indent();
	}

	indent(os_) << "}" << std::endl;

	dec_indent();

	return result(true, "Success");
}

result csharp_generator::generate_topic(const idl_type_topic* ttype, const std::string& id)
{
	WISE_UNUSED(ttype);

	auto ids = wise::kernel::split(id, ".");

	if (ids.size() != 3)
	{
		return result(false, "topic is invalid");
	}

	auto ns = ids[0]; // namespace
	auto em = ids[1]; // enum 
	auto typ = ids[2]; // type

	inc_indent();
	{
		indent(os_) << "public static Topic GetTopic() " << std::endl;
		indent(os_) << "{" << std::endl;

		inc_indent();
		{
			indent(os_) << "return new Topic( "
				<< "(byte)" << ns << "." << em << ".category, " 
				<< "(byte)" << ns << "." << em << ".group, " 
				<< "(ushort)" << ns << "." << em << "." << typ << " );" << std::endl;
		}
		dec_indent();

		indent(os_) << "}" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result csharp_generator::generate_field(const idl_field* field)
{
	auto ttype = field->get_type();

	if (!ttype)
	{
		WISE_ERROR("Field does not have a type. field: {}", field->get_variable_name());
		return result(false, "Field does not have a type");
	}

	WISE_INFO(
		"\tGenerating field. type: {}, varable: {}", 
		ttype->get_name(), field->get_variable_name()
	);

	switch (ttype->get_type())
	{
	case idl_type::type::simple:
		return generate_field_simple_type(field);

	case idl_type::type::full: 
		return generate_field_full_type(field);

	case idl_type::type::vec:
		return generate_field_vec(field);

	case idl_type::type::macro:
		return generate_field_macro(os_, field);

	case idl_type::type::topic:
		return result(true, "Topic is generated already.");

	case idl_type::type::option:
		return result(true, "Skip option field");

	default: 
		return result(false, "Field has unknown type");
	}
}

result csharp_generator::generate_field_simple_type(const idl_field* field)
{
	auto ttype = static_cast<const idl_type_simple*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a simple type"));

	inc_indent();
	{
		indent(os_) << "public " << ttype->get_typename_in("c#");

		os_ << " " << field->get_variable_name();

		auto fv = field->get_field_value();

		if (fv && fv->get_default_expression())
		{
			os_ << " = ";

			auto rc = generate_expression(field, fv->get_default_expression());
			WISE_RETURN_IF(!rc, rc);
		}
		else
		{
			if (ttype->get_simple_type() == idl_type_simple::types::TYPE_STRING ||
				ttype->get_simple_type() == idl_type_simple::types::TYPE_USTRING)
			{
				os_ << " = \"\"";
			}
		}

		os_ << ";" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result csharp_generator::generate_field_full_type(const idl_field* field)
{
	auto ttype = static_cast<const idl_type_full*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a full type"));

	inc_indent();
	{
		indent(os_) << "public " << ttype->get_typename_in("c#");

		os_ << " " << field->get_variable_name();

		if (!is_enum(field))
		{
			os_ << " = new " << ttype->get_typename_in("c#") << "()";
		}

		os_ << ";" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result csharp_generator::generate_field_vec(const idl_field* field)
{
	auto ttype = static_cast<const idl_type_vec*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a vec type"));

	auto vtype = ttype->get_value_type();
	WISE_ASSERT(vtype);

	inc_indent();
	{
		if ( vtype->get_type() == idl_type::type::simple)
		{ 
			auto stype = static_cast<const idl_type_simple*>(vtype);

			indent(os_) << "public List<" << stype->get_typename_in("c#") << "> ";
		}
		else
		{
			indent(os_) << "public List<" << vtype->get_name() << "> ";
		}

		os_ << field->get_variable_name();

		if ( vtype->get_type() == idl_type::type::simple)
		{ 
			auto stype = static_cast<const idl_type_simple*>(vtype);

			os_ << " = new List<" << stype->get_typename_in("c#") << ">()";
		}
		else
		{
			os_ << " = new List<" << vtype->get_name() << ">()";
		}

		os_ << ";" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result csharp_generator::generate_field_macro(std::ostream& os, const idl_field* field)
{
	auto ttype = static_cast<const idl_type_macro*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a macro type"));

	os << ttype->get_line();
	os << std::endl;

	return result(true, "Success");
}

result csharp_generator::generate_expression(const idl_field* field, const idl_expression* expr)
{
	// value op value op value ...

	auto ev = expr->get_value();

	generate_expression_value(field, ev);

	auto exprs = expr->get_exprs();

	// 파싱이 동작하도록 한 후에 작업 진행

	for (auto& ex : exprs)
	{
		auto op = ex->get_operator();

		// operator
		{
			auto rc = generate_operator(op);
			WISE_RETURN_IF(!rc, rc);
		}

		// next value
		{
			auto rc = generate_expression(field, ex);
			WISE_RETURN_IF(!rc, rc);
		}
	}

	return result(true, "Success");
}

result csharp_generator::generate_operator(idl_expression::Op op)
{
	switch (op)
	{
	case idl_expression::Op::Plus:
	{
		os_ << " + ";
	}
	break;
	case idl_expression::Op::Minus:
	{
		os_ << " - ";
	}
	break;
	default:
	{
		return result(false, "Invalid expression operator");
	}
	}

	return result(true, "Success");
}

result csharp_generator::generate_expression_value(const idl_field* field, const idl_expression_value* value)
{
	WISE_UNUSED(field);

	switch (value->get_type())
	{
	case idl_expression_value::Type::Const:
	{
		os_ << value->get_value();
	}
	break; 
	case idl_expression_value::Type::Simple:
	{
		os_ << value->get_id(); 
	}
	break; 
	case idl_expression_value::Type::Full: 
	{
		auto ft = value->get_full_type();
		os_ << ft->get_typename_in("c++");
	}
	break;
	default: 
	{
		return result(false, "Invalid value type");
	}
	break;
	}

	return result(true, "Success");
}

result csharp_generator::generate_prolog()
{
	os_ << "using System;" << std::endl;
	os_ << "using System.IO;" << std::endl;
	os_ << "using System.Collections.Generic;" << std::endl;
	os_ << "using wise;" << std::endl;
	os_ << std::endl;

	return result(true, "Success");
}

result csharp_generator::generate_namespace_begin(std::ostream& os)
{
	auto ns = program_->get_namespace();

	if (ns)
	{
		auto ns_node = static_cast<const idl_node_namespace*>(ns);
		auto lst = ns_node->get_namespace();

		for (size_t i = 0; i < lst.size(); ++i)
		{
			os << "namespace " << lst[i] << " {" << std::endl;
		}
	}
	else
	{
		os << "namespace wise" << std::endl;
		os << "{" << std::endl;
	}

	return result(true, "Success");
}

result csharp_generator::generate_namespace_end(std::ostream& os)
{
	auto ns = program_->get_namespace();

	os << std::endl;

	if (ns)
	{
		auto ns_node = static_cast<const idl_node_namespace*>(ns);
		auto lst = ns_node->get_namespace();

		for (size_t i = 0; i < lst.size(); ++i)
		{
			os << "} // "  << lst[i] << std::endl;
		}
	}
	else
	{
		os << "} // wise" << std::endl;
	}

	return result(true, "Success");
}

result csharp_generator::generate_epilog()
{
	os_ << std::endl;

	return result(true, "Success");
}
