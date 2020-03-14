%{ 

#define YYDEBUG 1			// debug 

//warning C4065: switch statement contains 'default' but no 'case' labels
#pragma warning(disable:4065)
//warning C4702: non-reachable code
#pragma warning(disable:4702)
// C4244: '인수': 'int64_t'에서 'int'(으)로 변환하면서 데이터가 손실될 수 있습니다.
#pragma warning(disable:4244)

#include "idl_main.h"
#include <wise/base/logger.hpp>

%}

/** 
 * make verbose error to debug
 */
%error-verbose

/**
 * This structure is used by the parser to hold the data types associated with
 * various parse nodes.
 */
%union {
  char*						id;
  const char*				dtext;
  const char*				vline;
  const char*				query;
  int64_t					iconst;
  double					dconst;
  bool						tbool;
  idl_enum_value*			tenumv;
  idl_field*				tfield;
  idl_field_value*			tfieldv;
  idl_expression* 			texpr;
  idl_type*					ttype;
  idl_node_enum*			tenum;
  idl_node_struct*			tstruct;
  idl_node_message*			tmessage;
  idl_node_namespace*		tnamespace;
  idl_node_include*			tinclude;
  idl_node_tx* 				ttx;
  tx_topic* 				ttx_topic;
  tx_bind* 					ttx_bind;
  tx_result* 				ttx_result;
  tx_result_set* 			ttx_result_set;
  tx_query*					ttx_query;
}

/**
 * Strings identifier
 */
%token<id>     tok_identifier
%token<id>     tok_literal

/**
 * Constant values
 */
%token<iconst> tok_int_constant
%token<dconst> tok_dub_constant

/**
 * Base datatype keywords
 */
%token tok_bool
%token tok_string
%token tok_ustring
%token tok_i8
%token tok_i16
%token tok_i32
%token tok_i64
%token tok_u8
%token tok_u16
%token tok_u32
%token tok_u64
%token tok_float
%token tok_double
%token tok_timestamp

/**
 * Syntatic symbols
 */
 %token tok_namespace_separator
 %token tok_namespace

/**
 * Language keywords
 */
%token tok_struct
%token tok_enum
%token tok_const
%token tok_message
%token tok_msg
%token tok_include
%token<vline> tok_macro_line
%token tok_vec
%token tok_topic
%token tok_out
%token tok_enable_cipher
%token tok_enable_checksum 
%token tok_enable_sequence
%token tok_opt

/** 
 * Query keywords 
 */
%token tok_tx 
%token tok_tx_bind 
%token tok_tx_result 
%token tok_tx_single 
%token tok_tx_multi
%token tok_tx_query
%token tok_tx_return
%token<query>     tok_query_string


/**
 * Grammar nodes
 */
%type<tenum>     	Enum
%type<tenum>     	EnumDefList
%type<tenumv>    	EnumDef
%type<tenumv>    	EnumValue

%type<tstruct>     	Struct
%type<tstruct>     	StructFieldList
%type<tmessage>		Message	
%type<tmessage>		MessageFieldList 
%type<tfield>   	Field 	
%type<tfieldv>   	FieldValue 	

%type<ttype> 		BasicType
%type<ttype> 		SimpleType
%type<ttype> 		FullType
%type<ttype> 		VecType
%type<ttype> 		NameAccessor
%type<ttype> 		SuperClassOptional
%type<texpr> 		SimpleExpression
%type<texpr> 		InnerExpression	
%type<tnamespace> 	Namespace
%type<tnamespace> 	NamespaceTail
%type<tinclude> 	Include
%type<tinclude> 	IncludePath

/** 
 * Grammar tx 
 */
%type<ttx>					Tx
%type<ttx>					TxNodeList
%type<ttx_topic>			TxNodeTopic
%type<ttx_bind>	    		TxNodeBind
%type<ttx_bind>	    		TxNodeBindFieldList
%type<ttx_query>			TxNodeQuery
%type<ttx_result>			TxNodeResult
%type<ttx_result>			TxNodeResultSetList
%type<ttx_result_set>		TxNodeResultSetField
%type<ttx_result_set>		TxNodeResultSetSingle
%type<ttx_result_set>		TxNodeResultSetMulti
%type<ttx_result_set>		TxNodeResultSetFieldList

%%

Program:
	DefinitionList 
	{
		WISE_DEBUG("Program => DefinitionList");
	}


DefinitionList:
  DefinitionList Definition
    {
		WISE_DEBUG("DefinitionList => DefinitionList Definition");
    }
|
    {
		WISE_DEBUG("DefinitionList => ");
    }

Definition: 
	Include
	{
		WISE_DEBUG("Include");
	}
|
	Namespace 
	{
		WISE_DEBUG("Namespace");
	}
|
	TypeDefinition 
    {
		WISE_DEBUG("Definition => TypeDefinition");
    }

Include: 
	tok_include '\"'  IncludePath tok_identifier '\"'
	{
		$$ = $3; 
		$$->add_path($4);

		g_program->add_node($$);
	}

IncludePath: 
	IncludePath tok_identifier '/' 
	{
		$$ = $1; 
		$$->add_path($2);
	}
|
	{
		$$ = new idl_node_include();
	}

Namespace: 
	tok_namespace NamespaceTail tok_identifier SemicolonOptional
	{
		WISE_DEBUG("tok_namespace NamespaceTail tok_identifier SemicolonOptional");

		$$ = $2;	
		$$->add_namespace($3);

		g_program->add_node($$);
	}

NamespaceTail: 
	NamespaceTail tok_identifier tok_namespace_separator
	{
		$$ = $1; 
		$$->add_namespace($2);
	}
| 
	{
		$$ = new idl_node_namespace();
	}
	
TypeDefinition:
	Enum
    {
		WISE_DEBUG("TypeDefinition => Enum");

		g_program->add_node($1);
    }
|	Struct 
	{
		WISE_DEBUG("TypeDefinition => Struct");

		g_program->add_node($1);
	}
|	Message
	{
		WISE_DEBUG("TypeDefinition => Message");

		g_program->add_node($1);
	}
|	Tx	
	{
		WISE_DEBUG("TypeDefinition => Tx");

		g_program->add_node($1);
	}

Enum:
  tok_enum tok_identifier '{' EnumDefList '}' SemicolonOptional
    {
		WISE_DEBUG("Enum => tok_enum tok_identifier '{' EnumDefList '}' SemicolonOptional");

		$$ = $4; 
		$$->set_name($2);

		g_symbols->add( 
			idl_symbol{
				idl_symbol::Enum, 
				g_program->get_fullname($$->get_name()), 
				$$->get_name(), 
				$$
			} 
		);
	}

EnumDefList:
  EnumDefList EnumDef
    {
		WISE_DEBUG("EnumDefList => EnumDefList EnumDef");

		$$ = $1;
		$$->add_value($2);
    }
|
    {
		WISE_DEBUG("EnumDefList => ");

		$$ = new idl_node_enum();
    }

EnumDef:
  EnumValue CommaOrSemicolonOptional
    {
		WISE_DEBUG("EnumDef => EnumValue CommaOrSemicolonOptional");

		$$ = $1;
    }

EnumValue:
  tok_identifier '=' SimpleExpression
    {
		WISE_DEBUG("EnumValue => tok_identifier '=' SimpleExpression");

		$$ = new idl_enum_value();
		$$->set_name($1);
		$$->set_default_expression($3);
    }
 |
  tok_identifier
    {
		WISE_DEBUG("EnumValue => tok_identifier");

		$$ = new idl_enum_value();
		$$->set_name($1);
    }

Struct:
  StructHead tok_identifier '{' StructFieldList '}' SemicolonOptional 
    {
		WISE_DEBUG("Struct => StructHead tok_identifier '{' StructFieldList '}' SemicolonOptional"); 

		$$ = $4;
		$$->set_name($2);

		g_symbols->add( 
			idl_symbol{
				idl_symbol::Struct, 
				g_program->get_fullname($$->get_name()), 
				$$->get_name(),
				$$
			} 
		);
    }

StructHead:
  tok_struct
    {
		WISE_DEBUG("StructHead => tok_struct");
    }


StructFieldList:
  StructFieldList Field
    {
		WISE_DEBUG("StructFieldList => StructFieldList Field");

		$1->add_field($2);
		$$ = $1;
    }
|
    {
		WISE_DEBUG("FieldList => ");

		// epsilon 매칭이 가장 먼저 실행된다. 왜?  

		$$ = new idl_node_struct();
    }

Message: 
  MessageKeyword tok_identifier SuperClassOptional '{' MessageFieldList '}' SemicolonOptional 
    {
		WISE_DEBUG("Message => tok_message tok_identifier '{' MessageBody '}' SuperClassOptional SemicolonOptional"); 

		$$ = $5;
		$$->set_name($2);
		$$->set_super_class($3);

		g_symbols->add( 
			idl_symbol{
				idl_symbol::Message, 
				g_program->get_fullname($$->get_name()), 
				$$->get_name(), 
				$$
			} 
		);
	}

MessageKeyword: 
	tok_message 
|   tok_msg


MessageFieldList:
	MessageFieldList Field
    {
		WISE_DEBUG("MessageFieldList => MessageFieldList Field");

		$1->add_field($2);
		$$ = $1;
    }
|
    {
		WISE_DEBUG("MessageFieldList => ");

		$$ = new idl_node_message();
    }

SuperClassOptional: 
	':' FullType
{
	$$ = $2;
}
|  
{
	$$ = nullptr;
}

Tx: 
  tok_tx tok_identifier '{' TxNodeList '}' SemicolonOptional 
    {
		WISE_DEBUG("Tx => tok_tx tok_identifier '{' TxFieldList '}' SemicolonOptional"); 

		$$ = $4;
		$$->set_name($2);
	}

TxNodeList: 
  	TxNodeList TxNodeTopic
	{
		WISE_DEBUG("TxNodeList => TxNoideList TxNodeTopic");

		$$ = $1; 
		$$->set_topic($2);
	}

| TxNodeList TxNodeBind 
	{
		WISE_DEBUG("TxNodeList => TxNoideList TxNodeBind");

		$$ = $1; 
		$$->set_bind($2);
	}
| TxNodeList TxNodeResult 
	{
		WISE_DEBUG("TxNodeList => TxNoideList TxNodeResult");

		$$ = $1;
		$$->set_result($2);
	}
| TxNodeList TxNodeQuery
	{
		WISE_DEBUG("TxNodeList => TxNodeList TxNodeQuery");

		$$ = $1; 
		$$->set_query($2);
	}
| 
	{
		WISE_DEBUG("TxNodeList => ");
		
		$$ = new idl_node_tx();
	}
 
TxNodeTopic: 
 	tok_topic tok_identifier CommaOrSemicolonOptional
	{
		WISE_DEBUG("TxNodeTopic => tok_topic tok_identifier CommaOrSemicolonOptional");

		$$ = new tx_topic();

		auto* pic = new idl_type_topic();
		pic->set_identifier($2);

		$$->set_topic(pic);
	}

TxNodeBind: 
	tok_tx_bind '{' TxNodeBindFieldList '}' CommaOrSemicolonOptional
	{
		WISE_DEBUG("TxNodeBind => tok_tx_bind '{' TxNodeBindFieldList '}' CommaOrSemicolonOptional");

		$$ = $3;
	}

TxNodeBindFieldList: 
	TxNodeBindFieldList Field
    {
		WISE_DEBUG("TxNodeBindFieldList => TxNodeBindFieldList Field");

		$$ = $1; 
		$$->add_field($2);
    }
| TxNodeBindFieldList tok_tx_return tok_identifier SemicolonOptional
    {
		WISE_DEBUG("TxNodeBindFieldList => TxNodeBindFieldList tok_tx_return tok_identifier");

		$$ = $1; 
		$$->set_return($3);
    }
|
    {
		WISE_DEBUG("TxNodeBindFieldList => ");

		$$ = new tx_bind();
    }


TxNodeResult: 
	tok_tx_result '{' TxNodeResultSetList '}' SemicolonOptional 
	{
		WISE_DEBUG("tok_tx_result '{' TxNodeResultSetList '}' SemicolonOptional");

		$$ = $3;
	}

TxNodeResultSetList: 
	TxNodeResultSetList TxNodeResultSetField 
	{
		WISE_DEBUG("TxNodeResultSetList => TxNodeResultSetList TxNodeResultSetField");

		$$ = $1;
		$$->add_result_set($2);
	}
| 	
	{
		WISE_DEBUG("TxNodeResultSetList =>");

		$$ = new tx_result();
	}

TxNodeResultSetField: 
	TxNodeResultSetSingle
	{
		WISE_DEBUG("TxNodeResultSetField => TxNodeResultSetSingle");

		$$ = $1;
	}
|   TxNodeResultSetMulti
	{
		WISE_DEBUG("TxNodeResultSetField => TxNodeResultSetMulti");

		$$ = $1;
	}


TxNodeResultSetSingle: 
	tok_tx_single '{' TxNodeResultSetFieldList '}' CommaOrSemicolonOptional 
	{
		WISE_DEBUG("tok_tx_single '{' TxNodeResultSetFieldList '}' CommaOrSemicolonOptional");

		$$ = $3;
		$$->set_single();
	}


TxNodeResultSetMulti: 
	tok_tx_multi '{' TxNodeResultSetFieldList '}' CommaOrSemicolonOptional 
	{
		WISE_DEBUG("tok_tx_multi '{' TxNodeResultSetFieldList '}' CommaOrSemicolonOptional");
		$$ = $3;
		$$->set_multi();
	}

TxNodeResultSetFieldList: 
	TxNodeResultSetFieldList Field
    {
		WISE_DEBUG("TxNodeResultSetFieldList => TxNodeBindFieldList Field");
		
		$$= $1;
		$$->add_field($2);
    }
|
    {
		WISE_DEBUG("TxNodeResultSetFieldList => ");

		$$ = new tx_result_set();
    }

TxNodeQuery: 
	tok_tx_query tok_query_string CommaOrSemicolonOptional
	{
		WISE_DEBUG("TxNodeQuery => tok_tx_query tok_query_string");

		$$ = new tx_query();
		$$->set_query($2);
	}

Field:
  BasicType tok_identifier FieldValue CommaOrSemicolonOptional
    {
		WISE_DEBUG("Field => BasicType tok_identifier FieldValue CommaOrSemicolonOptional");

		// create a field 
		$$ = new idl_field();
		$$->set_type($1);
		$$->set_variable_name($2);
		$$->set_field_value($3);
    }
| VecType tok_identifier CommaOrSemicolonOptional
    {
		WISE_DEBUG("Field => VecType tok_identifier CommaOrSemicolonOptional");

		$$ = new idl_field();
		$$->set_type($1);
		$$->set_variable_name($2);
    }
| tok_topic tok_identifier CommaOrSemicolonOptional
	{
		WISE_DEBUG("Field => tok_topic tok_identifier CommaOrSemicolonOptional");

		$$ = new idl_field();
		
		auto* topic_type = new idl_type_topic();
		topic_type->set_identifier($2);

		$$->set_type(topic_type);
	}
| tok_opt '(' tok_enable_cipher ')' CommaOrSemicolonOptional
{
	WISE_DEBUG("Field => tok_opt ( tok_enable_cipher ) CommaOrSemicolonOptional");

	$$ = new idl_field();
	
	auto* opt_type = new idl_type_opt(); 
	opt_type->set_option( idl_type_opt::opt::enable_cipher );

	$$->set_type(opt_type);
}
| tok_opt '(' tok_enable_sequence ')' CommaOrSemicolonOptional
{
	WISE_DEBUG("Field => tok_opt ( tok_enable_sequence ) CommaOrSemicolonOptional");

	$$ = new idl_field();

	auto* opt_type = new idl_type_opt(); 
	opt_type->set_option( idl_type_opt::opt::enable_sequence );

	$$->set_type(opt_type);
}
| tok_opt '(' tok_enable_checksum ')' CommaOrSemicolonOptional
{
	WISE_DEBUG("Field => tok_opt ( tok_enable_checksum ) CommaOrSemicolonOptional");

	$$ = new idl_field();

	auto* opt_type = new idl_type_opt(); 
	opt_type->set_option( idl_type_opt::opt::enable_checksum );

	$$->set_type(opt_type);
}
| tok_macro_line 

	{
		WISE_DEBUG("tok_macro_line {}", $1);

		$$ = new idl_field(); 

		auto* macro_type = new idl_type_macro();
		macro_type->set_line($1);

		$$->set_type(macro_type);
	}

BasicType: 
	FullType 
	{
		WISE_DEBUG("FullTpye {}", $1->get_name());

		$$ = $1;
	}
|   SimpleType
	{
		WISE_DEBUG("SimpleType {}", $1->get_name());

		$$ = $1;
	}


FullType: 
  NameAccessor tok_identifier 
	{
		WISE_DEBUG("FullType => NameAccessor tok_identifier");

		$$ = $1; 

		auto ftype = static_cast<idl_type_full*>($$);
		ftype->set_id($2); 
	}

NameAccessor: 
  NameAccessor tok_identifier tok_namespace_separator 
	{
		WISE_DEBUG("NameAccessor => NameAccessor tok_identifier tok_namespace_separator");

		$$ = $1;

		auto ftype = static_cast<idl_type_full*>($$);

		ftype->add_namespace($2); 
	}
| 
	{
		$$ = new idl_type_full(); 
	} 

VecType: 
  tok_vec '<' BasicType '>' 
	{
		WISE_DEBUG("VecType => tok_vec BasicType");

		auto vec_type = new idl_type_vec();
		vec_type->set_value_type( $3 );

		$$ = vec_type;
	}
| tok_vec '<' BasicType ',' tok_int_constant '>' 
	{
		WISE_DEBUG("VecType => tok_vec BasicType, count");

		auto vec_type = new idl_type_vec();
		vec_type->set_value_type( $3 );
		vec_type->set_count($5);

		$$ = vec_type;
	}

SimpleType:
  tok_string
    {
		WISE_DEBUG("SimpleType => tok_string");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_STRING); 
    }
| tok_ustring
    {
		WISE_DEBUG("SimpleType => tok_ustring");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_USTRING); 
    }
| tok_bool
    {
		WISE_DEBUG("SimpleType => tok_bool");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_BOOL); 
    }
| tok_i8
    {
		WISE_DEBUG("SimpleType => tok_i8");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_I8); 
    }
| tok_i16
    {
		WISE_DEBUG("SimpleType => tok_i16");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_I16); 
    }
| tok_i32
    {
		WISE_DEBUG("SimpleType => tok_i32");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_I32); 
    }
| tok_i64
    {
		WISE_DEBUG("SimpleType => tok_i64");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_I64); 
    }
| tok_u8
    {
		WISE_DEBUG("SimpleType => tok_u8");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_U8); 
    }
| tok_u16
    {
		WISE_DEBUG("SimpleType => tok_u16");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_U16); 
    }
| tok_u32
    {
		WISE_DEBUG("SimpleType => tok_u32");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_U32); 
    }
| tok_u64
    {
		WISE_DEBUG("SimpleType => tok_u64");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_U64); 
    }
| tok_float
    {
		WISE_DEBUG("SimpleType => tok_float");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_FLOAT); 
    }
| tok_double
    {
		WISE_DEBUG("SimpleType => tok_double");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_DOUBLE); 
    }
| tok_timestamp
    {
		WISE_DEBUG("SimpleType => tok_timestamp");

		$$ = new idl_type_simple(idl_type_simple::types::TYPE_TIMESTAMP); 
    }

FieldValue:
  '=' SimpleExpression
    {
		WISE_DEBUG("FieldValue => '=' SimpleExpression");

		$$ = new idl_field_value(); 
		$$->set_default_expression($2);
    }
| ':' tok_out
	{
		WISE_DEBUG("FieldValue => ':' tok_out");

		$$ = new idl_field_value(); 
		$$->set_output();
	}
|
    {
		WISE_DEBUG("FieldValue =>"); 

		$$ = new idl_field_value();
    }

SimpleExpression: 
   SimpleExpression '+' InnerExpression
	{
		WISE_DEBUG("SimpleExpression => SimpleExpression '+' InnerExpression");

		$$ = $1; 
		$$->add_plus($3);
	}
| SimpleExpression '-' InnerExpression
	{
		WISE_DEBUG("SimpleExpression => SimpleExpression '-' InnerExpression");

		$$ = $1; 
		$$->add_minus($3);
	}
| InnerExpression
	{
		$$ = $1;
	}

InnerExpression:
  FullType 
	{
		WISE_DEBUG("InnerExpression => FullType '.'  tok_identifier");

		$$ = new idl_expression(); 
		auto fv = new idl_expression_value(); 

		auto ftype = static_cast<idl_type_full*>($1);
		fv->set_full_id(ftype);

		$$->set_value(fv);
	}
| '+' tok_int_constant
	{
		WISE_DEBUG("'+' => tok_int_constant");

		$$ = new idl_expression(); 
		auto cv = new idl_expression_value(); 
		cv->set_const_value($2);
		$$->set_value(cv);
	}
| '-' tok_int_constant 
	{
		WISE_DEBUG("'-' => tok_int_constant");

		$$ = new idl_expression(); 
		auto cv = new idl_expression_value(); 
		cv->set_const_value(0-$2);
		$$->set_value(cv);
	}
| tok_int_constant 
	{
		WISE_DEBUG("InnerExpression => tok_int_constant");

		$$ = new idl_expression(); 
		auto cv = new idl_expression_value(); 
		cv->set_const_value($1);
		$$->set_value(cv);
	}



CommaOrSemicolonOptional:
  ','
    {
	}
| ';'
    {
	}
|
    {
	}
	
SemicolonOptional:
 ';'
    {}
|
    {}

%%
