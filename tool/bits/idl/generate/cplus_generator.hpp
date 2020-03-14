#pragma once 

#include "generator.hpp"
#include "../parse/idl_type_topic.h"
#include "../parse/idl_node_tx.h"
#include "../parse/tx/tx_bind.h"
#include "../parse/tx/tx_result_set.h"

class cplus_generator : public generator
{
public: 
	cplus_generator(idl_program* prog, std::ostream& os, std::ostream& oscpp, std::ostream& ossql)
		: generator(prog, os)
		, oscpp_(oscpp)
		, ossql_(ossql)
	{
	}

	result generate() override;

private: 
	result generate_nodes(); 

	result generate_factory();

	result generate_include(const idl_node* node);

	result generate_enum(const idl_node* node); 

	result generate_struct(const idl_node* node);

	result generate_struct_serialize(const idl_node* node);

	result generate_struct_pack(const idl_node* node);

	result generate_struct_unpack(const idl_node* node);

	result generate_message(const idl_node* node);
	
	result generate_message_topic(const idl_node* node);

	result generate_message_pack(const idl_node* node);

	result generate_message_unpack(const idl_node* node);

	result generate_tx(const idl_node* node);

	result generate_tx_decl(const idl_node* node);

	result generate_tx_impl(const idl_node* node);

	result generate_tx_topic(const idl_node* node);

	result generate_tx_bind_decl(const tx_bind* bind);

	result generate_tx_result_set_decl(std::size_t index, const tx_result_set* rs);

	result generate_tx_serialize_decl(const idl_node* node);

	result generate_tx_query_string(const idl_node* node);

	result generate_tx_execute(const idl_node* node);

	result generate_tx_execute_bind(const idl_node* node);

	result generate_tx_execute_bind_simple(tx_gen_context& ctx, const idl_field* field);

	result generate_tx_execute_bind_vec(tx_gen_context& ctx, const idl_field* field);

	result generate_tx_execute_bind_full(tx_gen_context& ctx, const idl_field* field);

	result generate_tx_execute_result(const idl_node* node);

	result generate_tx_serialize_impl(const idl_node* node);

	result generate_tx_field_serialize(const std::string& cls, const std::vector<idl_field*>& fields, const std::string& tx_return);

	result generate_topic(const idl_type_topic* ttype, const std::string& id);

	result generate_field(const idl_field* field);

	result generate_field_simple_type(const idl_field* field);

	result generate_field_full_type(const idl_field* field);

	result generate_field_vec(const idl_field* field);

	result generate_field_map(const idl_field* field);

	result generate_field_macro(std::ostream& os, const idl_field* field);

	result generate_expression(const idl_field* field, const idl_expression* expr);

	result generate_operator(idl_expression::Op op);

	result generate_expression_value(const idl_field* field, const idl_expression_value* value);

	result generate_prolog();

	result generate_hx_prolog();

	result generate_namespace_begin(std::ostream& os);

	result generate_namespace_end(std::ostream& os);

	result generate_epilog();

	result generate_cpp_prolog();

	result generate_cpp_epilog();

private: 
	std::ostream& oscpp_;	// tx 생성 시 cpp 코드 생성용 스트림
	std::ostream& ossql_;	// tx 생성 시 sql 코드 생성용 스트림
	tx_gen_context tx_ctx_;
};