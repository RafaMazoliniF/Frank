#include <stdio.h>
#include <stdlib.h>
#include "sintatico.h"

static FILE *file;
static Token current_token;

void parse_program() {
    if (current_token.symbol == SPROGRAMA) {
        free(current_token.lexeme);
        current_token = get_next_token(file);

        if (current_token.symbol == SIDENTIFICADOR) {
            free(current_token.lexeme);
            current_token = get_next_token(file);

            if (current_token.symbol == SPONTO_VIRGULA) {
                free(current_token.lexeme);
                current_token = get_next_token(file);

                parse_block();

                if (current_token.symbol == SPONTO) {
                    free(current_token.lexeme);
                    return;
                } else {
                    printf("Devia ser '.'\n");
                    exit(EXIT_FAILURE);
                }

            } else {
                printf("Devia ser ';'\n");
                exit(EXIT_FAILURE);
            }

        } else {
            printf("Devia ser um identificador \n");
            exit(EXIT_FAILURE);
        }

    } else {
        printf("Devia ser 'programa'\n");
        exit(EXIT_FAILURE);
    }
}

void parse_block() {
    free(current_token.lexeme);
    current_token = get_next_token(file);

    parse_variable_declaration_section();
    parse_subroutine_section();
    parse_commands();

    return true;
}

void parse_variable_declaration_section(){
    if (current_token.symbol == SVAR) {
        free(current_token.lexeme);
        current_token = get_next_token(file);

        if (current_token.symbol == SIDENTIFICADOR) {
            while (current_token.symbol == SIDENTIFICADOR) {
                parse_variables(); 

                if (current_token.symbol == SPONTO_VIRGULA) {
                    free(current_token.lexeme);
                    current_token = get_next_token(file);
                } else {
                    printf("Devia ser ';'\n");
                    exit(EXIT_FAILURE);
                }
            }

        } else {
            printf("Devia ser um identificador\n");
            exit(EXIT_FAILURE);
        }
    }
}

void parse_variables() {
    if (current_token.symbol != SIDENTIFICADOR) {
        printf("Devia ser um identificador\n");
        exit(EXIT_FAILURE);
    }

    while (current_token.symbol == SIDENTIFICADOR) {
        free(current_token.lexeme);
        current_token = get_next_token(file);

        if (current_token.symbol == SVIRGULA) {
            free(current_token.lexeme);
            current_token = get_next_token(file);

            if (current_token.symbol == SDOISPONTOS) {
                printf("Devia ser ':'\n");
                exit(EXIT_FAILURE);
            }

        } else if (current_token.symbol == SDOISPONTOS) {
            break; 
        } else {
            printf("Devia ser ',' ou ':'\n");
            exit(EXIT_FAILURE);
        }
    }

    free(current_token.lexeme);
    current_token = get_next_token(file);
    parse_type();
}

void parse_type() {
    if (current_token.symbol == SINTEIRO || current_token.symbol == SBOOLEANO) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devia ser um 'inteiro' ou 'booleano'\n");
        exit(EXIT_FAILURE);
    }
}

void parse_subroutine_section() {
    while (current_token.symbol == SPROCEDIMENTO || current_token.symbol == SFUNCAO) {
        if (current_token.symbol == SPROCEDIMENTO) {
            parse_procedure_declaration();  
        } else if (current_token.symbol == SFUNCAO) {
            parse_function_declaration();
        }

        if (current_token.symbol == SPONTO_VIRGULA) {
            free(current_token.lexeme);
            current_token = get_next_token(file);
        } else {
            printf("Devia ser ';'\n");
            exit(EXIT_FAILURE);
        }
    }
}

void parse_procedure_declaration() {
    if (current_token.symbol == SPROCEDIMENTO) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devia ser 'procedimento'\n");
        exit(EXIT_FAILURE);
    }

    if (current_token.symbol == SIDENTIFICADOR) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devia ser um identificador\n");
        exit(EXIT_FAILURE);
    }

    if (current_token.symbol == SPONTO_VIRGULA) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("DEvia ser ';'\n");
        exit(EXIT_FAILURE);
    }

    parse_block();
}

void parse_function_declaration() {
    if (current_token.symbol == SFUNCAO) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devia ser 'funcao'\n");
        exit(EXIT_FAILURE);
    }

    if (current_token.symbol == SIDENTIFICADOR) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devia ser um identificadot\n");
        exit(EXIT_FAILURE);
    }

    if (current_token.symbol == SDOISPONTOS) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devia se ':'\n");
        exit(EXIT_FAILURE);
    }

    parse_type();

    if (current_token.symbol == SPONTO_VIRGULA) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devia der ';'\n");
        exit(EXIT_FAILURE);
    }

    parse_block();
}

void parse_commands() {
    if (current_token.symbol == SINICIO) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devia ser 'inicio'\n");
        exit(EXIT_FAILURE);
    }

    parse_simple_command();

    while (current_token.symbol == SPONTO_VIRGULA) {
        free(current_token.lexeme);
        current_token = get_next_token(file);

        if (current_token.symbol == SFIM) {
            break;
        }
        parse_simple_command();
    }

    if (current_token.symbol == SFIM) {
        free(current_token.lexeme);
        current_token = get_next_token(file);
    } else {
        printf("Devai ser 'fim'\n");
        exit(EXIT_FAILURE);
    }
}

void parse_simple_command() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            parse_procedure_call();
            break;

        case SSE:
            parse_if_statement();
            break;

        case SENQUANTO:
            parse_while_statement();
            break;

        case SLEIA:
            parse_read_statement();
            break;

        case SESCREVA: 
            parse_write_statement();
            break;

        case SINICIO: 
            parse_commands();
            break;

        default:
            printf("Erro geral\n");
            exit(EXIT_FAILURE);
    }
}

void parser(FILE *filename) {
    file = filename;
    
    current_token = get_next_token(file);
    
    parse_program();

    if (current_token.lexeme != NULL) {
        printf("Erro");
        free(current_token.lexeme);
        exit(EXIT_FAILURE);
    }
}

