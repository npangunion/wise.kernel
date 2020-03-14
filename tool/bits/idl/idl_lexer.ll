%{

#pragma warning(disable: 4005) // INT_*에 대한 중복 정의 
#include "idl_main.h"
#include "idl_parser.hpp"
#include <wise/base/logger.hpp>

void integer_overflow(char* text) {
  yyerror("This integer is too big: \"%s\"\n", text);
  exit(1);
}

%}
/**
 * Provides the yylineno global, useful for debugging output
 */
%option lex-compat

/**
 * Our inputs are all single files, so no need for yywrap
 */
%option noyywrap

/**
 * We don't use it, and it fires up warnings at -Wall
 */
%option nounput

/**
 * Helper definitions, comments, constants, and whatnot
 */

intconstant		([0-9]+)
hexconstant		("0x"[0-9A-Fa-f]+)
dubconstant		([0-9]*(\.[0-9]+)?([eE][+-]?[0-9]+)?)
identifier		([a-zA-Z_](\.[a-zA-Z_0-9]|[a-zA-Z_0-9])*)
whitespace		([ \t\r\n]*)
comment			("//"[^\n]*)
symbol			([/\":;\.\,\{\}\(\)\=<>\[\]])
multicm_begin	("/*")
macro_line		("#"[^\n]*)
query_string	(\[([a-zA-Z_0-9]|\.|\[|\])*\])


%% 

{whitespace}        { /* do nothing */					}
{comment}			{ /* do nothing */					}

"::"				{ return tok_namespace_separator;	/* should come before symbol */ }
{symbol}            { return yytext[0];					}


"bool"              { return tok_bool;					}
"i8"                { return tok_i8;					}
"i16"               { return tok_i16;					}
"i32"               { return tok_i32;					}
"i64"               { return tok_i64;					}
"u8"                { return tok_u8;					}
"u16"               { return tok_u16;					}
"u32"               { return tok_u32;					}
"u64"               { return tok_u64;					}
"float"             { return tok_float;					}
"double"            { return tok_double;				}
"timestamp"			{ return tok_timestamp;				}
"string"            { return tok_string;				}
"ustring"           { return tok_ustring;				}
"vec"				{ return tok_vec;					}
"topic"				{ return tok_topic;					}
"enum"              { return tok_enum;					}
"struct"            { return tok_struct;				}
"message"           { return tok_message;				}
"msg"				{ return tok_msg;					}
[+]					{ return yytext[0];					}
[-]					{ return yytext[0];					}
"namespace"			{ return tok_namespace;				}
"include"			{ return tok_include;				}				

"opt"				{ return tok_opt;					}				
"enable_cipher"		{ return tok_enable_cipher;			}				
"enable_checksum"	{ return tok_enable_checksum;		}				
"enable_sequence"	{ return tok_enable_sequence;		}				

"tx"				{ return tok_tx;					}
"tx_bind"			{ return tok_tx_bind;				}
"tx_result"			{ return tok_tx_result;				}
"tx_single"			{ return tok_tx_single;				}
"tx_multi"			{ return tok_tx_multi;				}
"tx_query"			{ return tok_tx_query;				}
"tx_out"			{ return tok_out;					}
"tx_return"			{ return tok_tx_return;				}


{multicm_begin}  { /* parsed, but thrown away */
  std::string parsed("/*");
  int state = 0;  // 0 = normal, 1 = "*" seen, "*/" seen
  while(state < 2)
  {
    int ch = yyinput();
    parsed.push_back(static_cast<char>(ch));
    switch (ch) {
      case EOF:
        yyerror("Unexpected end of file in multiline comment at %d\n", yylineno);
        exit(1);
      case '*':
        state = 1;
        break;
      case '/':
        state = (state == 1) ? 2 : 0;
        break;
      default:
        state = 0;
        break;
    }
  }

  WISE_DEBUG("multi_comm = {}", parsed);
}

{intconstant} {
  errno = 0;
  yylval.iconst = strtoll(yytext, NULL, 10);
  if (errno == ERANGE) {
    integer_overflow(yytext);
  }
  return tok_int_constant;
}

{hexconstant} {
  errno = 0;
  char sign = yytext[0];
  int shift = sign == '0' ? 2 : 3;
  yylval.iconst = strtoll(yytext+shift, NULL, 16);
  if (sign == '-') {
    yylval.iconst = -yylval.iconst;
  }
  if (errno == ERANGE) {
    integer_overflow(yytext);
  }
  return tok_int_constant;
}

{identifier} {
  yylval.id = _strdup(yytext);
  return tok_identifier;
}

{macro_line} {
  yylval.vline = _strdup(yytext);
  WISE_DEBUG("verbatim line {}", yylval.vline);
  return tok_macro_line;
}

{dubconstant} {
 /* Deliberately placed after identifier, since "e10" is NOT a double literal (THRIFT-3477) */
  yylval.dconst = atof(yytext);
  return tok_dub_constant;
}

{query_string} {
  yylval.query = _strdup(yytext); 
  return tok_query_string;
}

. {
  WISE_ERROR("Unexpected token: {}", yytext);
}

%%