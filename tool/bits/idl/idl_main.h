#pragma once 

#include "parse/idl_field.h"
#include "parse/idl_expression.h"
#include "parse/idl_type_simple.h"
#include "parse/idl_type_full.h"
#include "parse/idl_type_map.h"
#include "parse/idl_type_vec.h"
#include "parse/idl_type_topic.h"
#include "parse/idl_type_macro.h"
#include "parse/idl_node_struct.h"
#include "parse/idl_node_enum.h"
#include "parse/idl_node_message.h"
#include "parse/idl_node_namespace.h"
#include "parse/idl_node_include.h"
#include "parse/idl_node_tx.h"
#include "parse/idl_program.h"
#include "idl_symbol_table.hpp"

#include <cassert>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h> /* for GetFullPathName */
#endif

extern "C" { int yylex(void); }

int yyparse(void);

/**
* Expected to be defined by Flex/Bison
*/
void yyerror(const char* fmt, ...);


extern int yydebug;
extern int yylineno;
extern char yytext[];
extern std::FILE* yyin;
extern std::string g_curpath;

extern idl_program* g_program;

extern idl_symbol_table* g_symbols;

/** 
 * parse a file. 
 */
int parse_file(const std::string& file);

/** 
 * parse a program
 */
int parse_program(idl_program::ptr program);

/**
 * generate a program
 */
bool generate_program(idl_program::ptr program);
