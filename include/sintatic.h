#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "lexical.h"

typedef struct Token {
    Simbolo symbol;
    char * lexem;
} Token;

Token current_token;
void get_next_token();

bool handle_simple_expression();
bool handle_term();
bool handle_factor();
bool handle_variable();
bool handle_function_call();
bool handle_identifier();
bool handle_number();