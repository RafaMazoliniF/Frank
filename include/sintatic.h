#ifndef SINTATIC_H
#define SINTATIC_H

#include "lexical.h"
#include "includes.h"
#include "semantic.h"

void handler();
void handle_program();
void handle_block();
int handle_variable_declaration_section();
void handle_variables(int * count);
void handle_type();
void handle_commands();
void handle_command();
void handle_procedure_call(Token aux);
void handle_read_statement();
void handle_write_statement();
void handle_while_statement();
void handle_if_statement();
bool handle_subroutine_section();
void handle_procedure_declaration();
void handle_function_declaration();
DataType handle_expression();
DataType handle_simple_expression();
DataType handle_term();
DataType handle_factor();
void handle_variable(SymbolNode * node);
void handle_function_call(SymbolNode * node);
void handle_identifier();
void handle_number();

void handle_assignment_chprocedure();
void handle_assignment_command(Token aux);
void handle_conditional_command();
void handle_while_command();
void handle_read_command();
void handle_write_command();
void handle_relational_operator();

#endif