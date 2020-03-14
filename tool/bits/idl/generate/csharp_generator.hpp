#pragma once 

#include "generator.hpp"
#include "../parse/idl_type_topic.h"
#include "../parse/idl_node_tx.h"
#include "../parse/tx/tx_bind.h"
#include "../parse/tx/tx_result_set.h"

/// cplus message source generator 
/** 
 * C# message support: 
 * - enum 
 * - struct 
 * - message
 * - fields 
 *   - u*, i*, float, double 
 *   - string, ustring, vec
 * 
 * Copied from C++ implementation and modified for C#. 
 * For other client langs, begin from C# implementation.
 */
class csharp_generator : public generator
{
public: 
	csharp_generator(idl_program* prog, std::ostream& os)
		: generator(prog, os)
	{
	}

	result generate() override;

private: 
	result generate_nodes(); 

	result generate_factory();

	result generate_include(const idl_node* node);

	result generate_enum(const idl_node* node); 

	result generate_struct(const idl_node* node);

	result generate_struct_pack(const idl_node* node);

	result generate_struct_unpack(const idl_node* node);

	result generate_message(const idl_node* node);
	
	result generate_message_topic(const idl_node* node);

	result generate_message_pack(const idl_node* node);

	result generate_message_unpack(const idl_node* node);

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

	result generate_namespace_begin(std::ostream& os);

	result generate_namespace_end(std::ostream& os);

	result generate_epilog();

private: 
	tx_gen_context tx_ctx_;
};