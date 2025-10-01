#ifndef SYNTACTIC_H
#define SYNTACTIC_H

#include "lexical.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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
bool handle_simple_expression();
bool handle_term();
bool handle_factor();
bool handle_variable();
bool handle_function_call();
bool handle_identifier();
bool handle_number();

typedef struct Token {
    Simbolo symbol;
    char * lexem;
} Token;

Token current_token;
void get_next_token();

void assignment_or_procedure_call();
void assignment_command();
void procedure_call();
void conditional_command();
void while_command();
void read_command();
void write_command();
void expression();
void relational_operator();

#endif
