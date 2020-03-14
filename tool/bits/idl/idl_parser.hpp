/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_IDL_IDL_PARSER_HPP_INCLUDED
# define YY_YY_IDL_IDL_PARSER_HPP_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tok_identifier = 258,
     tok_literal = 259,
     tok_int_constant = 260,
     tok_dub_constant = 261,
     tok_bool = 262,
     tok_string = 263,
     tok_ustring = 264,
     tok_i8 = 265,
     tok_i16 = 266,
     tok_i32 = 267,
     tok_i64 = 268,
     tok_u8 = 269,
     tok_u16 = 270,
     tok_u32 = 271,
     tok_u64 = 272,
     tok_float = 273,
     tok_double = 274,
     tok_timestamp = 275,
     tok_namespace_separator = 276,
     tok_namespace = 277,
     tok_struct = 278,
     tok_enum = 279,
     tok_const = 280,
     tok_message = 281,
     tok_msg = 282,
     tok_include = 283,
     tok_macro_line = 284,
     tok_vec = 285,
     tok_topic = 286,
     tok_out = 287,
     tok_enable_cipher = 288,
     tok_enable_checksum = 289,
     tok_enable_sequence = 290,
     tok_opt = 291,
     tok_tx = 292,
     tok_tx_bind = 293,
     tok_tx_result = 294,
     tok_tx_single = 295,
     tok_tx_multi = 296,
     tok_tx_query = 297,
     tok_tx_return = 298,
     tok_query_string = 299
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 26 "idl/idl_parser.yy"

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


/* Line 2058 of yacc.c  */
#line 128 "idl\\\\idl_parser.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_IDL_IDL_PARSER_HPP_INCLUDED  */
