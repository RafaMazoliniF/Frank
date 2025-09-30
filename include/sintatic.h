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

void assignment_or_procedure_call();
void assignment_command();
void procedure_call();
void conditional_command();
void while_command();
void read_command();
void write_command();
void expression();
void relational_operator();