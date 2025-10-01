#ifndef SYNTACTIC_H
#define SYNTACTIC_H

#include "lexical.h"

void parser(FILE *filename);

void parse_program();
void parse_block();
void parse_variable_declaration_section();
void parse_variables();
void parse_type();
void parse_commands();
void parse_simple_command();
void parse_procedure_call();
void parse_read_statement();
void parse_write_statement();
void parse_while_statement();
void parse_if_statement();
void parse_subroutine_section();
void parse_procedure_declaration();
void parse_function_declaration();
void parse_expression();
void parse_simple_expression();
void parse_term();
void parse_factor();

#endif
