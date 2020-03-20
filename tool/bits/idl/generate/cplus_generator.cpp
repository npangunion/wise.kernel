#include <pch.hpp>
#include "cplus_generator.hpp"
#include <idl/idl_symbol_table.hpp>
#include <idl/parse/idl_program.h>
#include <idl/parse/idl_node.h>
#include <idl/parse/idl_node_include.h>
#include <idl/parse/idl_node_enum.h>
#include <idl/parse/idl_node_struct.h>
#include <idl/parse/idl_node_message.h>
#include <idl/parse/idl_node_tx.h>
#include <idl/parse/idl_node_namespace.h>
#include <idl/parse/idl_type_full.h>
#include <idl/parse/idl_type_macro.h>
#include <idl/parse/idl_type_map.h>
#include <idl/parse/idl_type_simple.h>
#include <idl/parse/idl_type_topic.h>
#include <idl/parse/idl_type_vec.h>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/macros.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

result cplus_generator::generate()
{
	auto rc = generate_nodes();

	if (rc && (program_->has_message() || program_->has_tx()) )
	{
		rc = generate_factory();
	}

	return rc;
}

result cplus_generator::generate_nodes()
{
	WISE_EXPECT(program_);

	WISE_INFO("Generating file: {}", program_->get_path());

	// prolog
	{
		auto rc = generate_prolog();
		WISE_RETURN_IF(!rc, rc);

		if (program_->has_tx())
		{
			auto rc2 = generate_cpp_prolog(); 
			WISE_RETURN_IF(!rc2, rc2);
		}

		if (program_->has_hx())
		{
			generate_hx_prolog();
		}
	}

	// include
	{
		bool gen = false;
		auto& nodes = program_->get_nodes();

		for (auto& node : nodes)
		{
			if (node->get_type() == idl_node::Include)
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
			case idl_node::Enum:
			{
				auto rc = generate_enum(node);
				WISE_RETURN_IF(!rc, rc);
			}
			break; 
			case idl_node::Struct: 
			{
				auto rc = generate_struct(node);
				WISE_RETURN_IF(!rc, rc);
			}
			break; 
			case idl_node::Message:
			{
				auto rc = generate_message(node);
				WISE_RETURN_IF(!rc, rc);
			}
			break;
			case idl_node::Tx:
			{
				auto rc = generate_tx(node);
				WISE_RETURN_IF(!rc, rc);
			}
			break;
			case idl_node::Include:
			{
				// ignore
			}
			break;
			case idl_node::Namespace:
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

	if ( program_->has_struct())
	{
		os_ << "// struct serialization section" << std::endl;
		os_ << "namespace wise { " << std::endl;
		os_ << "namespace kernel { " << std::endl;

		// TODO: generate struct serialize code in wise namespace
		for (auto& node : nodes)
		{
			if (node->get_type() == idl_node::Struct)
			{
				auto rc = generate_struct_serialize(node);
				WISE_RETURN_IF(!rc, rc);
			}
		}

		os_ << std::endl;
		os_ << "} // kernel" << std::endl;
		os_ << "} // wise" << std::endl;
	}

	// epilog 
	{
		auto rc = generate_epilog();
		WISE_RETURN_IF(!rc, rc);

		if (program_->has_tx())
		{
			auto rc2 = generate_cpp_epilog(); 
			WISE_RETURN_IF(!rc2, rc2);
		}
	}

	WISE_INFO("Completed {}", program_->get_path());

	return result(true, "Success");
}

result cplus_generator::generate_factory()
{
	WISE_EXPECT(program_);

	//  _factory.cpp 파일에 추가
	// 

	namespace fs = boost::filesystem;

	std::string ext_removed = wise::kernel::remove_file_extension(program_->get_path());

	fs::path er_file(ext_removed);

	std::string filename = wise::kernel::get_filename(ext_removed);

	// header file
	{
		auto hfile = ext_removed;

		hfile.append("_factory.hpp");

		std::ofstream hos(hfile.c_str());
	
		hos << "#pragma once" << std::endl;
		hos << std::endl;

		hos << "void add_" << filename << "();"; 
	}

	// cpp file
	{
		auto hfile = ext_removed;
		hfile.append("_factory.cpp");

		std::ofstream os(hfile.c_str());

		fs::path inc(program_->get_path());
		inc.replace_extension(".hpp");

		// TODO: make it optional 

		os << "#include \"pch.hpp\"" << std::endl;
		os << "#include <wise.kernel/net/protocol/bits/bits_factory.hpp>" << std::endl;
		os << "#include <wise.kernel/core/mem_tracker.hpp>" << std::endl;
		os << "#include \"" << inc.string() << "\"";
		os << std::endl;
		os << std::endl;

		os << "#define WISE_ADD_BITS(cls) wise::kernel::bits_factory::inst().add( "
			"cls::get_topic(), []() { return wise_shared<cls>(); "
			"} );";

		os << std::endl;
		os << std::endl;

		os << "void add_" << filename << "()" << std::endl; 
		os << "{" << std::endl;

		auto& nodes = program_->get_nodes();

		inc_indent();

		for (auto& node : nodes)
		{
			if (node->get_type() == idl_node::Message)
			{
				indent(os) << "WISE_ADD_BITS(" << get_namespace() 
					<< node->get_name() << ");" 
					<< std::endl;
			}
			else if (node->get_type() == idl_node::Tx)
			{
				indent(os) << "WISE_ADD_BITS(" << get_namespace() 
					<< node->get_name() << ");" 
					<< std::endl;
			}
		}

		dec_indent();

		os << "}" << std::endl;
	}

	return result(true, "Success");
}

result cplus_generator::generate_include(const idl_node* node)
{
	auto in = static_cast<const idl_node_include*>(node);

	std::string path = in->get_path_string();

	WISE_EXPECT(path.size() > 3);

	auto np = path.substr(0, path.size() - 4);

	np.append(".hpp");

	// replace trailing .r2 to .h

	os_ << "#include \"" << np << "\"" << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_enum(const idl_node* node)
{
	auto en = static_cast<const idl_node_enum*>(node);

	os_ << std::endl;
	os_ << "enum class " << en->get_name() << std::endl;
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

	os_ << "};" << std::endl;
	os_ << std::endl;


	return result(true, "Success");
}

result cplus_generator::generate_struct(const idl_node* node)
{
	auto sn = static_cast<const idl_node_struct*>(node);

	os_ << std::endl;
	os_ << "struct " << sn->get_name() << std::endl;
	os_ << "{" << std::endl;

	// field 생성 
	auto fields = sn->get_fields();

	for (auto& field : fields)
	{
		auto rc = generate_field(field);

		WISE_RETURN_IF(!rc, rc);
	}

	os_ << "};" << std::endl;
	os_ << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_struct_serialize(const idl_node* node)
{
	node;
	// NOTE: macro는 그대로 포함하면서 처리해야 한다. 

	os_ << std::endl;

	os_ << "// " << get_namespace() << node->get_name() 
		<< " serialization begin { " 
		<< std::endl;

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

	os_ << "// } " << get_namespace() << node->get_name() 
		<< " serialization end" 
		<< std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_struct_pack(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	indent(os_) << "template<> inline bool pack(bits_packer& packer, "; 
	os_ << "const " << get_namespace() << node->get_name() << "& tv) " << std::endl;
	indent(os_) << "{" << std::endl;

	auto fields = sn->get_fields();

	inc_indent();

	for (auto& field : fields)
	{
		if (is_serializable_field(field))
		{
			if (is_enum(field))
			{
				indent(os_) << "packer.pack_enum(" "tv." 
					<< field->get_variable_name() << ");" 
					<< std::endl;
			}
			else
			{
				indent(os_) << "packer.pack(" << "tv." 
					<< field->get_variable_name() << ");" 
					<< std::endl;
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

	indent(os_) << "return packer.is_valid();" << std::endl;

	dec_indent();

	indent(os_) << "}" << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_struct_unpack(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	indent(os_) << "template<> inline bool unpack(bits_packer& packer, ";
	os_ << get_namespace() << node->get_name() << "& tv) " << std::endl;
	indent(os_) << "{" << std::endl;

	auto fields = sn->get_fields();

	inc_indent();

	for (auto& field : fields)
	{
		if (is_serializable_field(field))
		{
			if (is_enum(field))
			{
				indent(os_) << "packer.unpack_enum(" << "tv." 
					<< field->get_variable_name() << ");" 
					<< std::endl;
			}
			else
			{
				indent(os_) << "packer.unpack(" << "tv." 
					<< field->get_variable_name() << ");" 
					<< std::endl;
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

	indent(os_) << "return packer.is_valid();" << std::endl;

	dec_indent();

	indent(os_) << "}" << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_message(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	os_ << std::endl;

	std::string super_class = "wise::kernel::bits_packet";

	auto sc = sn->get_super_class();
	if (sc)
	{
		auto scft = static_cast<const idl_type_full*>(sc);
		super_class = scft->get_typename_in("c++");
	}

	indent(os_) << "class " << sn->get_name() << " : public " << super_class << std::endl;
	indent(os_) << "{" << std::endl;

	// using
	{
		indent(os_) << "\tusing super = " << super_class << ";" << std::endl;
	}

	// field 생성 
	{
		indent(os_) << "public: // field variables " << std::endl;

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
		indent(os_) << "public: // topic and constructor" << std::endl;
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
		indent(os_) << node->get_name() << "()" << std::endl;
		indent(os_) << ": " << super_class << "(get_topic())" << std::endl;
		indent(os_) << "{" << std::endl;

		if (sn->is_set_enable_cipher())
		{
			indent(os_) << "\tenable_cipher = true;" << std::endl;
		}
		if (sn->is_set_enable_checksum())
		{
			indent(os_) << "\tenable_checksum = true;" << std::endl;
		}
		if (sn->is_set_enable_sequence())
		{
			indent(os_) << "\tenable_sequence = true;" << std::endl;
		}

		indent(os_) << "}" << std::endl;
	}

	dec_indent();

	os_ << std::endl;

	// serialize
	{
		indent(os_) << "public: // serialize" << std::endl;

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

	// message desc
	os_ << std::endl;
	os_ << "public: WISE_MESSAGE_DESC( " << sn->get_name() << " )" << std::endl;

	indent(os_) << "};" << std::endl;
	os_ << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_message_topic(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	auto fields = sn->get_fields();

	for (auto& field : fields)
	{
		auto ttype = field->get_type();

		if (ttype->get_type() == idl_type::topic)
		{
			const auto topic_type = static_cast<const idl_type_topic*>(ttype);
			auto id = topic_type->get_identifier();

			return generate_topic(topic_type, id);
		}
	}

	return result(false, "Topic field not found");
}

result cplus_generator::generate_message_pack(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	inc_indent();

	indent(os_) << "bool pack(::wise::kernel::bits_packer& packer) override" << std::endl;
	indent(os_) << "{" << std::endl;

	// fields
	{
		inc_indent();

		auto fields = sn->get_fields();

		indent(os_) << "super::pack(packer);" << std::endl;

		for (auto& field : fields)
		{
			if (is_serializable_field(field))
			{
				if (is_enum(field))
				{
					indent(os_) << "packer.pack_enum(" 
						<< field->get_variable_name() << ");" 
						<< std::endl;
				}
				else
				{
					indent(os_) << "packer.pack(" 
						<< field->get_variable_name() << ");" 
						<< std::endl;
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


		indent(os_) << "return packer.is_valid();" << std::endl;

		dec_indent();
	}

	indent(os_) << "}" << std::endl;

	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_message_unpack(const idl_node* node)
{
	auto sn = static_cast<const idl_node_message*>(node);
	WISE_ASSERT(sn);

	inc_indent();

	indent(os_) << "bool unpack(::wise::kernel::bits_packer& packer) override" << std::endl;
	indent(os_) << "{" << std::endl;

	// fields
	{
		inc_indent();

		indent(os_) << "super::unpack(packer);" << std::endl;

		auto fields = sn->get_fields();

		for (auto& field : fields)
		{
			if (is_serializable_field(field))
			{
				if (is_enum(field))
				{
					indent(os_) << "packer.unpack_enum(" 
						<< field->get_variable_name() << ");" 
						<< std::endl;
				}
				else
				{
					indent(os_) << "packer.unpack(" 
						<< field->get_variable_name() << ");" 
						<< std::endl;
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

		indent(os_) << "return packer.is_valid();" << std::endl;

		dec_indent();
	}

	indent(os_) << "}" << std::endl;

	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_tx(const idl_node* node)
{
	// decl
	{
		auto rc = generate_tx_decl(node);
		WISE_RETURN_IF(!rc, rc);
	}

	return generate_tx_impl(node);
}

result cplus_generator::generate_tx_impl(const idl_node* node)
{
	reset_indent();

	auto sn = static_cast<const idl_node_tx*>(node);
	WISE_ASSERT(sn);

	ossql_ << "-- tx " << node->get_name() << std::endl;

	// execute
	{
		auto rc = generate_tx_execute(node);
		WISE_RETURN_IF(!rc, rc);
	}

	// tx serialize 
	{
		auto rc = generate_tx_serialize_impl(node);
		WISE_RETURN_IF(!rc, rc);
	}

	oscpp_ << std::endl;

	// bind 
	{
		auto bind = sn->get_bind();
		WISE_RETURN_IF(!bind, result(false, "bind is missing in tx defintion"));

		auto fields = bind->get_fields();

		std::ostringstream oss; 

		oss << node->get_name() << "::bind_t";

		auto rc = generate_tx_field_serialize(oss.str(), fields, bind->get_return());
		WISE_RETURN_IF(!rc, rc);
	}

	oscpp_ << std::endl;

	// result
	{
		auto rs = sn->get_result();

		if (rs)
		{
			os_ << std::endl;

			auto sets = rs->get_results();

			for (std::size_t i = 0; i < sets.size(); ++i)
			{
				std::ostringstream oss; 

				oss << node->get_name() << "::" << "rs_" << i + 1 << "_t"; 

				generate_tx_field_serialize( oss.str(), sets[i]->get_fields(), "");
			}
		}
	}

	return result(true, "Success");
}

result cplus_generator::generate_tx_decl(const idl_node* node)
{
	auto sn = static_cast<const idl_node_tx*>(node);
	WISE_ASSERT(sn);

	os_ << std::endl;

	indent(os_) << "class " << sn->get_name() << " : public wise::kernel::tx" << std::endl;
	indent(os_) << "{" << std::endl;


	// query, topic
	{
		os_ << "public: // query, topic, constructor" << std::endl;

		auto rc = generate_tx_query_string(node);
		WISE_RETURN_IF(!rc, rc);

		os_ << std::endl;

		rc = generate_tx_topic(node);

		if (!rc)
		{
			WISE_ERROR("Fail to generate topic: {}", rc.value);
		}

		WISE_RETURN_IF(!rc, result(false, "Failed to generate topic"));
	}

	os_ << std::endl;

	// constructor / execute
	inc_indent();
	{
		// constructor
		indent(os_) << node->get_name() << "()" << std::endl;
		indent(os_) << ": wise::kernel::tx(get_topic())" << std::endl;
		indent(os_) << "{" << std::endl;
		indent(os_) << "\tquery_ = query;" << std::endl;
		indent(os_) << "}" << std::endl;

		// execute decl
		os_ << std::endl;

		indent(os_) << "result execute_query(dbc::statement::ptr stmt) override;" << std::endl;
	}
	dec_indent();

	// bind 
	{
		auto bind = sn->get_bind();
		WISE_RETURN_IF(!bind, result(false, "bind is missing in tx defintion"));

		auto rc = generate_tx_bind_decl(bind);
		WISE_RETURN_IF(!rc, rc);
	}

	// result
	{
		auto rs = sn->get_result();

		if (rs)
		{
			os_ << std::endl;

			indent(os_) << "public: // result set structure " << std::endl;

			auto sets = rs->get_results();

			for (std::size_t i = 0; i < sets.size(); ++i)
			{
				auto rc = generate_tx_result_set_decl(i, sets[i]);
				WISE_RETURN_IF(!rc, rc);
			}
		}
	}

	// pack / unpack 
	{
		auto rc = generate_tx_serialize_decl(node);
		WISE_RETURN_IF(!rc, rc);
	}

	// message desc
	os_ << std::endl;
	os_ << "public: WISE_MESSAGE_DESC( " << sn->get_name() << " )" << std::endl;

	indent(os_) << "};" << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_tx_topic(const idl_node* node)
{
	auto* sn = static_cast<const idl_node_tx*>(node);
	WISE_ASSERT(sn);

	const auto* txt = sn->get_topic();
	WISE_RETURN_IF(!txt, result(false, "Topic is null"));

	const auto* tt = txt->get_topic();
	WISE_ASSERT(tt);

	return generate_topic(tt, tt->get_identifier());
}

result cplus_generator::generate_tx_bind_decl(const tx_bind* bind)
{
	os_ << std::endl;

	indent(os_) << "public: // bind structure" << std::endl;

	inc_indent();
	{
		indent(os_) << "struct bind_t" << std::endl;
		indent(os_) << "{" << std::endl;

		// return 생성 
		{
			if (bind->has_return())
			{
				indent(os_) << "\tint32_t " << bind->get_return() << ";" << std::endl;
			}
		}

		// field 생성 
		{
			auto fields = bind->get_fields();

			for (auto& field : fields)
			{
				auto rc = generate_field(field);

				WISE_RETURN_IF(!rc, rc);
			}
		}

		os_ << std::endl;

		inc_indent();
		{
			indent(os_) << "bool pack(wise::kernel::bits_packer& packer); " << std::endl;
			indent(os_) << "bool unpack(wise::kernel::bits_packer& packer); " << std::endl;
		}
		dec_indent();

		indent(os_) << "} bind;" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_tx_result_set_decl(std::size_t index, const tx_result_set* rs)
{
	WISE_ASSERT(rs);

	os_ << std::endl;

	inc_indent();
	{
		indent(os_) << "struct rs_"  << index + 1 << "_t" << std::endl;
		indent(os_) << "{" << std::endl;

		// field 생성 
		{
			auto fields = rs->get_fields();

			for (auto& field : fields)
			{
				auto rc = generate_field(field);

				WISE_RETURN_IF(!rc, rc);
			}
		}

		os_ << std::endl;

		inc_indent();
		{
			indent(os_) << "bool pack(wise::kernel::bits_packer& packer); " << std::endl;
			indent(os_) << "bool unpack(wise::kernel::bits_packer& packer); " << std::endl;
		}
		dec_indent();

		indent(os_) << "};" << std::endl;
	}

	if (rs->is_single())
	{
		indent(os_) << "rs_" << index + 1 << "_t rs_" << index + 1 << ";" << std::endl;
	}
	else
	{
		indent(os_) << "std::vector<rs_" << index + 1 << "_t> rs_" << index + 1 << ";" << std::endl;
	}

	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_tx_serialize_decl(const idl_node* node)
{
	WISE_UNUSED(node);

	std::ostream& os = os_;

	os << std::endl;

	indent(os) << "public: // tx serialization" << std::endl;

	// pack
	inc_indent();
	{
		indent(os) << "bool pack(wise::kernel::bits_packer& packer) override;" << std::endl;
		indent(os) << "bool unpack(wise::kernel::bits_packer& packer) override;" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_tx_query_string(const idl_node* node)
{
	auto sn = static_cast<const idl_node_tx*>(node);

	auto qx = sn->get_query();
	WISE_RETURN_IF(qx == nullptr, result(false, "Tx query string is null"));

	auto& qs = qx->get_query();
	WISE_RETURN_IF(qs.length() < 3, result(false, "query string is too short"));

	std::string nqs = qs.substr(1, qs.length()-2);

	tx_ctx_.query = nqs;

	auto bind = sn->get_bind();
	WISE_RETURN_IF(bind == nullptr, result(false, "Bind is not defined"));
	
	auto& fields = bind->get_fields();

	std::ostringstream oss;

	if (bind->has_return())
	{
		oss << "? = ";
	}

	oss << "CALL " << nqs << "(" ;

	for (int i = 0; i < fields.size(); ++i)
	{
		if (i == 0)
		{
			oss << "?";
		}
		else
		{
			oss << ", ?";
		}
	}

	oss << ")";

	inc_indent();
	{
		indent(os_) << "static constexpr wchar_t* query = L\"{ " << oss.str() << " }\";" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_tx_execute(const idl_node* node)
{
	oscpp_ << std::endl;

	indent(oscpp_) << "wise::kernel::tx::result " 
		<< node->get_name() << "::execute_query(dbc::statement::ptr stmt)" 
		<< std::endl;

	indent(oscpp_) << "{" << std::endl;

	inc_indent();
	{
		auto rc = generate_tx_execute_bind(node);
		WISE_RETURN_IF(!rc, rc);
		oscpp_ << std::endl;

		indent(oscpp_) << "stmt->execute();" << std::endl;
		oscpp_ << std::endl;

		rc = generate_tx_execute_result(node);
		WISE_RETURN_IF(!rc, rc);
		oscpp_ << std::endl;

		indent(oscpp_) << "return result::success;" << std::endl;
	}
	dec_indent();

	indent(oscpp_) << "}" << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_tx_execute_bind(const idl_node* node)
{
	auto sn = static_cast<const idl_node_tx*>(node);

	auto bind = sn->get_bind();
	WISE_RETURN_IF(bind == nullptr, result(false, "bind is null in tx"));

	auto& fields = bind->get_fields();

	auto& ctx = tx_ctx_;

	ctx.bind_seq = 0;
	ctx.name = sn->get_name();

	ossql_ << "CREATE PROCEDURE " << ctx.query <<  std::endl;

	if (bind->has_return())
	{
		indent(oscpp_) << "stmt->bind( "
			<< ctx.bind_seq << ", " << "&bind."
			<< bind->get_return()
			<< ", dbc::statement::PARAM_OUT );"
			<< std::endl;

		ctx.bind_seq++;
	}

	for (int i = 0; i < fields.size(); ++i)
	{
		auto ft = fields[i]->get_type();

		WISE_INFO(
			"\tGenerating tx bind. type: {}, variable: {}",
			ft->get_name(), fields[i]->get_variable_name(), ft->get_type()
		);

		switch (ft->get_type())
		{
		case idl_type::simple:
		{
			auto rc = generate_tx_execute_bind_simple(ctx, fields[i]);
			WISE_RETURN_IF(!rc, rc);
		}
		break;
		case idl_type::vec:
		{
			auto rc = generate_tx_execute_bind_vec(ctx, fields[i]);
			WISE_RETURN_IF(!rc, rc);
		}
		break;
		case idl_type::full:
		{
			auto rc = generate_tx_execute_bind_full(ctx, fields[i]);
			WISE_RETURN_IF(!rc, rc);
		}
		break;
		case idl_type::macro:
		{
			(void)generate_field_macro(oscpp_, fields[i]);
		}
		break;
		case idl_type::map:
		case idl_type::topic:
		{
			WISE_ERROR("Tx supported bind type. name: {}", ft->get_name());
			return result(false, "Tx unsupported bind type");
		}
		break;
		default: 
		{
			WISE_ERROR("Tx supported bind type. name: {}", ft->get_name());
		}
		}
	}

	ossql_ << "AS " << std::endl;
	ossql_ << "BEGIN" << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_tx_execute_bind_simple(tx_gen_context& ctx, const idl_field* field)
{
	auto stype = static_cast<const idl_type_simple*>(field->get_type());

	std::string postfix;
	std::string prefix = "&";

	switch (stype->get_simple_type())
	{
	case idl_type_simple::TYPE_BOOL:
		postfix = "_bool";
		prefix = "";
		break;
	case idl_type_simple::TYPE_I8:
		postfix = "_int8";
		prefix = "";
		break;
	case idl_type_simple::TYPE_U8:
		postfix = "_uint8";
		prefix = "";
		break;
	}

	const auto& fv = field->get_field_value();

	if (fv != nullptr && fv->is_output())
	{
		if (stype->get_simple_type() == idl_type_simple::TYPE_STRING ||
			stype->get_simple_type() == idl_type_simple::TYPE_USTRING)
		{
			WISE_ERROR("Tx string does not support PARAM_OUT binding");
			return result(false, "Tx string does not support PARAM_OUT binding");
		}

		if (ctx.bind_seq > 0)
		{
			ossql_ << ", ";
		}

		ossql_ << "@" << field->get_variable_name() << " " << stype->get_typename_in("sql") << " OUT" << std::endl;

	
		indent(oscpp_) << "stmt->bind" << postfix << "( "
			<< ctx.bind_seq << ", " << prefix << "bind."
			<< field->get_variable_name()
			<< ", dbc::statement::PARAM_OUT );"
			<< std::endl;
	}
	else
	{
		if (ctx.bind_seq > 0)
		{
			ossql_ << ", ";
		}

		ossql_ << "@" << field->get_variable_name() << " " << stype->get_typename_in("sql") << std::endl;

		if (stype->get_simple_type() == idl_type_simple::TYPE_STRING ||
			stype->get_simple_type() == idl_type_simple::TYPE_USTRING)
		{
			indent(oscpp_) << "stmt->bind_strings( "
				<< ctx.bind_seq << ", "
				<< "bind." << field->get_variable_name() << ".c_str(), "
				<< "bind." << field->get_variable_name() << ".length(), 1 );"
				<< std::endl;
		}
		else
		{
			indent(oscpp_) << "stmt->bind" << postfix << "( "
				<< ctx.bind_seq << ", " << prefix << "bind."
				<< field->get_variable_name() << " );" <<
				std::endl;
		}
	}

	ctx.bind_seq++;

	return result(true, "Success");
}

result cplus_generator::generate_tx_execute_bind_vec(tx_gen_context& ctx, const idl_field* field)
{
	auto vt = static_cast<const idl_type_vec*>(field->get_type());

	oscpp_ << std::endl;

	// vec size check
	{
		indent(oscpp_) << "if ( " << "bind." << field->get_variable_name() << ".size() < " << vt->get_count() << ")" << std::endl;
		indent(oscpp_) << "{" << std::endl;

		indent(oscpp_) << "\tWISE_ERROR( \"tx invalid vector size. tx : " << ctx.name
			<< ", name: " << field->get_variable_name()
			<< ", count: " << vt->get_count() << "\" );"
			<< std::endl;

		indent(oscpp_) << "\treturn result::fail_invalid_vector_size;" << std::endl;
		indent(oscpp_) << "}" << std::endl;
	}

	int count = vt->get_count();

	auto vft = vt->get_value_type();

	switch (vft->get_type())
	{
	case idl_type::simple:
	{
		for (int i = 0; i < count; ++i)
		{
			idl_field sfield;
			sfield.set_type(const_cast<idl_type*>(vft));

			std::ostringstream oss;
			oss << field->get_variable_name() << "[" << i << "]";

			sfield.set_variable_name(oss.str());

			(void)generate_tx_execute_bind_simple(ctx, &sfield);

			sfield.forget_type();
		}
	}
	break;
	case idl_type::full:
	{
		for (int i = 0; i < count; ++i)
		{
			idl_field ffield;
			ffield.set_type(const_cast<idl_type*>(vft));

			std::ostringstream oss;
			oss << field->get_variable_name() << "[" << i << "]";

			ffield.set_variable_name(oss.str());

			auto rc = generate_tx_execute_bind_full(ctx, &ffield);
			WISE_RETURN_IF(!rc, rc);
			ffield.forget_type();
		}
	}
	break;
	case idl_type::vec:
	case idl_type::map:
	case idl_type::topic:
	case idl_type::macro:
	{
		WISE_ERROR("Tx not supported bind type in vector. name: {}", vft->get_name());
		return result(false, "Tx unsupported bind type in vector");
	}
	break;
	} // switch

	return result(true, "Success");
}

result cplus_generator::generate_tx_execute_bind_full(tx_gen_context& ctx, const idl_field* field)
{
	const auto& fv = field->get_field_value();

	if (fv != nullptr && fv->is_output())
	{
		WISE_ERROR("Tx bind for struct used for INPUT only!");
		return result(false, "Tx bind for struct used for INPUT only!");
	}

	auto ft = field->get_type();

	auto rc = g_symbols->lookup(ft->get_name());

	if (!rc)
	{
		rc = g_symbols->lookup( program_->get_namespace()->get_name(), ft->get_name() );
	}

	if (!rc)
	{
		WISE_ERROR("Tx failed to find a struct name {}", ft->get_name());
		return result(false, "Tx failed to find a struct name");
	}

	if (rc.value.type_ != idl_symbol::Struct)
	{
		WISE_ERROR(
			"Tx bind only supports struct type. name: {} var: {}", 
			ft->get_name(), field->get_variable_name()
		);

		return result(false, "Tx bind only supports struct type");
	}

	auto tn = rc.value.node;
	WISE_RETURN_IF(
		tn == nullptr, result(false, "Tx bind struct node pointer is null")
	);

	auto sn = static_cast<const idl_node_struct*>(tn);
	auto fields = sn->get_fields();

	for (int i = 0; i < fields.size(); ++i)
	{
		auto ft2 = fields[i]->get_type();

		if (ft2->get_type() != idl_type::simple)
		{
			WISE_ERROR(
				"Tx bind struct supports only simple types. name: {} var: {}",
				ft2->get_name(), fields[i]->get_variable_name()
			);

			return result(false, "Tx bind struct supports only simple types");
		}

		auto stype = static_cast<const idl_type_simple*>(ft2);

		if (ctx.bind_seq > 0)
		{
			ossql_ << ", ";
		}

		ossql_ << "@" << field->get_variable_name() << "." << fields[i]->get_variable_name()  
			<< " " << stype->get_typename_in("sql") << std::endl;

		if (stype->get_simple_type() == idl_type_simple::TYPE_STRING ||
			stype->get_simple_type() == idl_type_simple::TYPE_USTRING)
		{
			indent(oscpp_) << "stmt->bind_strings( "
				<< ctx.bind_seq << ", "
				<< "bind." << field->get_variable_name() << "." << fields[i]->get_variable_name() << ".c_str(), "
				<< "bind." << field->get_variable_name() << "." << fields[i]->get_variable_name() << ".length(), 1 );"
				<< std::endl;
		}
		else
		{
			indent(oscpp_) << "stmt->bind( "
				<< ctx.bind_seq << ", &"
				<< "bind." << field->get_variable_name() << "." << fields[i]->get_variable_name() << " );" 
				<< std::endl;
		}

		ctx.bind_seq++;
	}

	return result(true, "Success");
}

result cplus_generator::generate_tx_execute_result(const idl_node* node)
{
	auto sn = static_cast<const idl_node_tx*>(node);

	auto rx = sn->get_result();
	WISE_RETURN_IF(rx == nullptr, result(true, "Success with empty result set"));

	auto& rss = rx->get_results();
	
	for (int i = 0; i < rss.size(); ++i)
	{
		auto rs = rss[i];

		std::string rs_name; 

		// get name
		{
			std::ostringstream oss; 
			oss << "rs_" << i + 1;
			rs_name = oss.str();
		}

		indent(oscpp_) << "// rs " << i + 1 << std::endl;
		indent(oscpp_) << "{" << std::endl;

		inc_indent();
		{
			indent(oscpp_) << "auto& rs = stmt->next_result();" << std::endl;
			oscpp_ << std::endl;

			// begin
			if (rs->is_single())
			{
				indent(oscpp_) << "if ( rs.next() )" << std::endl;
				indent(oscpp_) << "{" << std::endl;
			}
			else
			{
				indent(oscpp_) << "while ( rs.next() )" << std::endl;
				indent(oscpp_) << "{" << std::endl;
			}

			inc_indent();
			{
				auto& fields = rs->get_fields();

				for (auto& field : fields)
				{
					if (field->get_type()->get_type() != idl_type::simple && 
						field->get_type()->get_type() != idl_type::macro )
					{
						WISE_ERROR(
							"Tx result set should have simple types or macro only. tx: {}, field: {}",
							node->get_name(), field->get_variable_name()
						);

						return result(false, "Tx result set should have simple types or macro only");
					}
				}


				if (rs->is_single())
				{
					int seq = 0;

					ossql_ << "-- " << rs_name << " single row result set" << std::endl;

					for (auto& field : fields)
					{
						if (field->get_type()->get_type() == idl_type::macro)
						{
							(void)generate_field_macro(oscpp_, field);
						}
						else
						{
							auto stype = static_cast<const idl_type_simple*>(field->get_type());

							ossql_ << rs_name << "." << field->get_variable_name() << " " << stype->get_typename_in("sql") << std::endl;

							indent(oscpp_) 
								<< "rs.get_ref( " << seq++ << ", "
								<< rs_name << "." << field->get_variable_name()
								<< " );"
								<< std::endl;
						}
					}
				}
				else
				{
					int seq = 0;

					indent(oscpp_) << rs_name << "_t rset;" << std::endl;

					ossql_ << "-- " << rs_name << " multiple row result set" << std::endl;

					for (auto& field : fields)
					{
						if (field->get_type()->get_type() == idl_type::macro)
						{
							(void)generate_field_macro(oscpp_, field);
						}
						else
						{
							auto stype = static_cast<const idl_type_simple*>(field->get_type());

							ossql_ << rs_name << "." << field->get_variable_name() << " " << stype->get_typename_in("sql") << std::endl;

							indent(oscpp_) << "rset." << field->get_variable_name()
								<< " = rs.get<" << stype->get_typename_in("c++") << ">"
								<< "( " << seq++ << " );"
								<< std::endl;
						}
					}

					indent(oscpp_) << rs_name << ".push_back(rset);" << std::endl;
				}
			}
			dec_indent();

			// end
			indent(oscpp_) << "}" << std::endl;
		}
		dec_indent();
		indent(oscpp_) << "}" << std::endl;
	}

	ossql_ << "END -- tx " << node->get_name() << std::endl;
	ossql_ << std::endl << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_tx_serialize_impl(const idl_node* node)
{
	std::ostream& os = oscpp_;

	auto sn = static_cast<const idl_node_tx*>(node);

	os << std::endl;

	// pack
	{
		indent(os) << "bool " << node->get_name() << "::pack(wise::kernel::bits_packer& packer)" << std::endl;
		indent(os) << "{" << std::endl;

		inc_indent();
		{
			indent(os) << "wise::kernel::tx::pack(packer);" << std::endl;
			indent(os) << "bind.pack(packer);" << std::endl;

			// result
			{
				auto rs = sn->get_result();

				if (rs)
				{
					indent(os) << "// result sets: " << std::endl;

					auto sets = rs->get_results();

					for (std::size_t i = 0; i < sets.size(); ++i)
					{
						if (sets[i]->is_single())
						{
							indent(os) << "rs_" << i + 1 << ".pack(packer);" << std::endl;
						}
						else
						{
							indent(os) << "uint16_t len = " << "rs_" << i + 1 << ".size();" << std::endl;
							indent(os) << "packer.pack(len);" << std::endl;

							indent(os) << "for ( auto& rs : " << "rs_" << i + 1 << ") " << std::endl;
							indent(os) << "{" << std::endl;
							inc_indent(); 
							{
								indent(os) << "rs_" << i + 1 << ".pack(packer); " << std::endl;
							}
							dec_indent();
							indent(os) << "}" << std::endl;
						}
					}
				}
			}

			indent(os) << "return packer.is_valid();" << std::endl;
		}
		dec_indent();

		indent(os) << "}";
	}

	os << std::endl;
	os << std::endl;

	// unpack
	{
		indent(os) << "bool " << node->get_name() << "::unpack(wise::kernel::bits_packer& packer)" << std::endl;
		indent(os) << "{" << std::endl;

		inc_indent();
		{
			indent(os) << "wise::kernel::tx::unpack(packer);" << std::endl;
			indent(os) << "bind.unpack(packer);" << std::endl;

			// result
			{
				auto rs = sn->get_result();

				if (rs)
				{
					indent(os) << "// result sets: " << std::endl;

					auto sets = rs->get_results();

					for (std::size_t i = 0; i < sets.size(); ++i)
					{
						if (sets[i]->is_single())
						{
							indent(os) << "rs_" << i + 1 << ".unpack(packer);" << std::endl;
						}
						else
						{
							indent(os) << "uint16_t len = 0" << std::endl;
							indent(os) << "packer.unpack(len);" << std::endl;

							indent(os) << "for ( uint16_t i=0; i < len; ++i) " << std::endl;
							indent(os) << "{" << std::endl;

							inc_indent(); 
							{
								indent(os) << "rs_" << i + 1 << "_t rs; " << std::endl;
								indent(os) << "rs.unpack(packer);" << std::endl;
								indent(os) << "rs_" << i + 1 << ".push_back(rs);" << std::endl;
							}
							dec_indent();
							indent(os) << "}" << std::endl;
						}
					}
				}
			}

			indent(os) << "return packer.is_valid();" << std::endl;
		}
		dec_indent();

		indent(os) << "}" << std::endl;
	}

	return result(true, "Success");
}

result cplus_generator::generate_tx_field_serialize(const std::string& cls, const std::vector<idl_field*>& fields, const std::string& tx_return)
{
	std::ostream& os = oscpp_;

	// pack 
	{
		indent(os) << "bool " << cls << "::pack(wise::kernel::bits_packer& packer)" << std::endl;
		indent(os) << "{" << std::endl;

		inc_indent();
		{
			if (!tx_return.empty())
			{
				indent(os) << "packer.pack(" << tx_return << ");" << std::endl;
			}

			for (auto& field : fields)
			{
				if (is_serializable_field(field))
				{
					if (is_enum(field))
					{
						indent(os) << "packer.pack_enum(" 
							<< field->get_variable_name() << ");" << std::endl;
					}
					else
					{
						indent(os) << "packer.pack("
							<< field->get_variable_name() << ");" << std::endl;
					}
				}
				else
				{
					if (is_macro(field))
					{
						generate_field_macro(os, field);
					}
				}
			}

			indent(os) << "return packer.is_valid();" << std::endl;
		}
		dec_indent();

		indent(os) << "}" << std::endl;
	}

	os << std::endl;

	//unpack
	{
		indent(os) << "bool " << cls << "::unpack(wise::kernel::bits_packer& packer)" << std::endl;
		indent(os) << "{" << std::endl;

		inc_indent();
		{
			if (!tx_return.empty())
			{
				indent(os) << "packer.unpack(" << tx_return << ");" << std::endl;
			}

			for (auto& field : fields)
			{
				if (is_serializable_field(field))
				{
					if (is_enum(field))
					{
						indent(os) << "packer.unpack_enum(" 
							<< field->get_variable_name() << ");" << std::endl;
					}
					else
					{
						indent(os) << "packer.unpack(" 
							<< field->get_variable_name() << ");" << std::endl;
					}
				}
				else
				{
					if (is_macro(field))
					{
						generate_field_macro(os, field);
					}
				}
			}

			indent(os) << "return packer.is_valid();" << std::endl;
		}
		dec_indent();

		indent(os) << "}" << std::endl;
	}

	return result(true, "Success");
}


result cplus_generator::generate_topic(const idl_type_topic* ttype, const std::string& id)
{
	WISE_UNUSED(ttype);

	auto ids = wise::kernel::split(id, ".");

	if (ids.size() != 3)
	{
		return result(false, "topic is invalid");
	}

	auto cat = ids[0];
	auto grp = ids[1];
	auto typ = ids[2];

	inc_indent();
	{
		indent(os_) << "static const wise::kernel::topic& get_topic() " << std::endl;
		indent(os_) << "{" << std::endl;

		inc_indent();
		{
			indent(os_) << "static wise::kernel::topic topic_( " << std::endl;

			inc_indent();
			{
				indent(os_) << "static_cast<wise::kernel::topic::category_t>(" 
					<< cat << "::" << grp << "::category), " 
					<< std::endl;

				indent(os_) << "static_cast<wise::kernel::topic::group_t>(" 
					<< cat << "::" << grp << "::group), " 
					<< std::endl;

				indent(os_) << "static_cast<wise::kernel::topic::type_t>(" 
					<< cat << "::" << grp << "::" << typ << ")" 
					<< std::endl;
			}
			dec_indent();

			indent(os_) << ");" << std::endl;

			os_ << std::endl;

			indent(os_) << "return topic_;" << std::endl;
		}
		dec_indent();

		indent(os_) << "}" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_field(const idl_field* field)
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
	case idl_type::simple:
		return generate_field_simple_type(field);

	case idl_type::full: 
		return generate_field_full_type(field);

	case idl_type::vec:
		return generate_field_vec(field);

	case idl_type::map:
		return generate_field_map(field);

	case idl_type::macro:
		return generate_field_macro(os_, field);

	case idl_type::topic:
		return result(true, "Topic is generated already.");

	case idl_type::option:
		return result(true, "skip");

	default: 
		return result(false, "Field has unknown type");
	}
}

result cplus_generator::generate_field_simple_type(const idl_field* field)
{
	auto ttype = static_cast<const idl_type_simple*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a simple type"));

	inc_indent();
	{
		indent(os_) << ttype->get_typename_in("c++");

		os_ << " " << field->get_variable_name();

		auto fv = field->get_field_value();

		if (fv && fv->get_default_expression())
		{
			os_ << " = ";

			auto rc = generate_expression(field, fv->get_default_expression());
			WISE_RETURN_IF(!rc, rc);
		}

		os_ << ";" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_field_full_type(const idl_field* field)
{
	auto ttype = static_cast<const idl_type_full*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a full type"));

	inc_indent();
	{
		indent(os_) << ttype->get_typename_in("c++");

		os_ << " " << field->get_variable_name();

		os_ << ";" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_field_vec(const idl_field* field)
{
	auto ttype = static_cast<const idl_type_vec*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a vec type"));

	auto vtype = ttype->get_value_type();
	WISE_ASSERT(vtype);

	inc_indent();
	{
		if ( vtype->get_type() == idl_type::simple)
		{ 
			auto stype = static_cast<const idl_type_simple*>(vtype);

			indent(os_) << "std::vector<" << stype->get_typename_in("c++") << "> ";
		}
		else
		{
			indent(os_) << "std::vector<" << vtype->get_name() << "> ";
		}

		os_ << field->get_variable_name();

		os_ << ";" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_field_map(const idl_field* field)
{
	auto ttype = static_cast<const idl_type_map*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a vec type"));

	auto vtype = ttype->get_value_type();
	WISE_ASSERT(vtype);

	auto ktype = ttype->get_key_type();
	WISE_ASSERT(ktype);

	inc_indent();
	{
		indent(os_) << "std::map<"; 

		if (ktype->get_type() == idl_type::simple)
		{
			auto stype = static_cast<const idl_type_simple*>(ktype);

			os_ << stype->get_typename_in("c++");
		}
		else
		{ 
			os_ << ktype->get_name();
		} 
		os_ << ", ";


		if (vtype->get_type() == idl_type::simple)
		{
			auto stype = static_cast<const idl_type_simple*>(vtype);

			os_ << stype->get_typename_in("c++");
		}
		else
		{ 
			os_ << vtype->get_name();
		} 

		os_ << "> ";

		os_ << field->get_variable_name();

		os_ << ";" << std::endl;
	}
	dec_indent();

	return result(true, "Success");
}

result cplus_generator::generate_field_macro(std::ostream& os, const idl_field* field)
{
	auto ttype = static_cast<const idl_type_macro*>(field->get_type());
	WISE_RETURN_IF(!ttype, result(false, "Field does not have a macro type"));

	os << ttype->get_line();
	os << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_expression(const idl_field* field, const idl_expression* expr)
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

result cplus_generator::generate_operator(idl_expression::Op op)
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

result cplus_generator::generate_expression_value(const idl_field* field, const idl_expression_value* value)
{
	switch (value->get_type())
	{
	case idl_expression_value::Type::Const:
	{
		os_ << value->get_value();

		if (field)
		{
			auto ttype = static_cast<const idl_type_simple*>(field->get_type());

			if (ttype && ttype->get_simple_type() == idl_type_simple::TYPE_FLOAT)
			{
				os_ << ".f";
			}
		}
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

result cplus_generator::generate_prolog()
{
	os_ << "#pragma once" << std::endl;
	os_ << "#include <wise/net/protocol/bits/bits_packet.hpp>" << std::endl;
	os_ << "#include <wise/net/protocol/bits/bits_packer.hpp>" << std::endl;
	os_ << std::endl;

	if (program_->has_tx())
	{
		os_ << "#include <wise/service/db/tx.hpp>" << std::endl;
		os_ << std::endl;
	}

	os_ << "#include <stdint.h>" << std::endl;
	os_ << "#include <map>" << std::endl;
	os_ << "#include <string>" << std::endl;
	os_ << "#include <vector>" << std::endl;
	os_ << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_hx_prolog()
{
	os_ << "#include <wise/server/service/hx.hpp>" << std::endl;
	os_ << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_namespace_begin(std::ostream& os)
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

result cplus_generator::generate_namespace_end(std::ostream& os)
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

result cplus_generator::generate_epilog()
{
	os_ << std::endl;

	return result(true, "Success");
}

result cplus_generator::generate_cpp_prolog()
{
	namespace fs = boost::filesystem;

	fs::path h(program_->get_path());
	h.replace_extension(".hpp");
	
	oscpp_ << "#include \"pch.hpp\"" << std::endl;
	oscpp_ << "#include \"" <<  h.string() << "\"" << std::endl;
	oscpp_ << "#include <wise.kernel/core/logger.hpp>" << std::endl;

	oscpp_ << std::endl;

	return generate_namespace_begin(oscpp_);
}

result cplus_generator::generate_cpp_epilog()
{
	return generate_namespace_end(oscpp_);
}

