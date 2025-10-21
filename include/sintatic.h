#ifndef SINTATIC_H
#define SINTATIC_H

#include "lexical.h"
#include "includes.h"

void handler();
void handle_program();
void handle_block();
void handle_variable_declaration_section();
void handle_variables();
void handle_type();
void handle_commands();
void handle_command();
void handle_procedure_call();
void handle_read_statement();
void handle_write_statement();
void handle_while_statement();
void handle_if_statement();
void handle_subroutine_section();
void handle_procedure_declaration();
void handle_function_declaration();
void handle_expression();
void handle_simple_expression();
void handle_term();
void handle_factor();
void handle_variable();
void handle_function_call();
void handle_identifier();
void handle_number();

void handle_assignment_chprocedure();
void handle_assignment_command();
void handle_procedure_call();
void handle_conditional_command();
void handle_while_command();
void handle_read_command();
void handle_write_command();
void handle_expression();
void handle_relational_operator();

#endif