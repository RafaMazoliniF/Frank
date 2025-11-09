#include "sintatic.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

void print_sintax_error(const char* expected) {
    fprintf(stderr, "ERRO SINTÁTICO na linha %d: ", current_line);
    if (expected != NULL) {
        fprintf(stderr, "esperado '%s', encontrado '%s'\n", expected, current_token.lexem);
    } else {
        fprintf(stderr, "token inesperado '%s'\n", current_token.lexem);
    }
    fflush(stderr);
    exit(EXIT_FAILURE);
}

// <programa>::= programa <identificador> ; <bloco> .
void handle_program() {
    get_next_token();

    if (current_token.symbol == SPROGRAMA) {
        get_next_token();

        if (current_token.symbol == SIDENTIFICADOR) {
            // Semantic action: Insert program name into symbol table.
            insert_node_table(current_token.lexem, true, PROGRAM_NAME, -1);
            get_next_token();

            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block();

                if (current_token.symbol == SPONTO) {
                    get_next_token();

                    if (current_token.lexem != NULL || current_token.symbol != ENDFILE) {
                        fprintf(stderr, "ERRO SINTÁTICO na linha %d: código após o fim do programa\n", current_line);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    }
                } else {
                    print_sintax_error(".");
                }
            } else {
                print_sintax_error(";");
            }
        } else {
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador do programa após 'programa'\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: programa deve começar com a palavra 'programa'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

// <bloco>::= [<etapa de declaração de variáveis>]
//           [<etapa de declaração de sub-rotinas>]
//           <comandos>
void handle_block() {
    handle_variable_declaration_section();
    handle_subroutine_section();
    handle_commands();
}

// <etapa de declaração de variáveis>::= var <declaração de variáveis> ;
//                                     {<declaração de variáveis>;}
void handle_variable_declaration_section(){
    if (current_token.symbol == SVAR) {
        get_next_token();

        if (current_token.symbol == SIDENTIFICADOR) {
            while (current_token.symbol == SIDENTIFICADOR) {
                handle_variables();

                if (current_token.symbol == SPONTO_VIRGULA) {
                    get_next_token();
                } else {
                    print_sintax_error(";");
                }
            }
        } else {
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador de variável após 'var'\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }
}

// <declaração de variáveis>::= <identificador> {, <identificador>} : <tipo>
void handle_variables() {
    do {
        if (current_token.symbol == SIDENTIFICADOR) {
            // Semantic action: Check if variable can be declared and insert into table.
            if (can_declare_variable(current_token.lexem)) {
                insert_node_table(current_token.lexem, false, VAR, -1);

                get_next_token();

                if (current_token.symbol == SVIRGULA) {
                    get_next_token();
                    if (current_token.symbol != SIDENTIFICADOR && current_token.symbol != SDOISPONTOS) {
                        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador após ','\n", current_line);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    }
                } else if (current_token.symbol == SDOISPONTOS) {
                    break;
                } else {
                    print_sintax_error(",' ou ':");
                }
            }
            else {
                // Semantic error: Variable already exists.
                fprintf(stderr, "ERRO SEMANTICO na linha %d: variável %s já existe\n", current_line, current_token.lexem);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            // ===================================
        } else {
            print_sintax_error("identificador");
        }
    } while (current_token.symbol != SDOISPONTOS);

    get_next_token();
    handle_type();
    // Semantic action: Apply type to all declared variables.
}


// <tipo> ::= (inteiro | booleano)
void handle_type() {
    if (current_token.symbol != SINTEIRO && current_token.symbol != SBOOLEANO) {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado tipo 'inteiro' ou 'booleano'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    else {
        // Semantic action: Insert type (INT or BOOL).
        if (current_token.symbol == SINTEIRO)
            insert_type(INT);
        else 
            insert_type(BOOL);
    }
    get_next_token();
}

// <comandos>::= inicio
//               <comando>{;<comando>}[;]
//              fim
void handle_commands() {
    if (current_token.symbol == SINICIO) {
        get_next_token();
        handle_command();

        while (current_token.symbol != SFIM) {
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                // Handle optional trailing semicolon before 'fim'
                if (current_token.symbol != SFIM) {
                    handle_command();
                }
            } else {
                // Error: token is not 'fim' and not ';', so it's a syntax error.
                print_sintax_error(";");
            }
        }
        // Consume 'fim' token.
        get_next_token();

    } else {
        print_sintax_error("inicio");
    }
}

// <comando>
void handle_command() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            handle_assignment_chprocedure();
            break;
        case SSE:
            handle_conditional_command();
            break;
        case SENQUANTO:
            handle_while_command();
            break;
        case SLEIA:
            handle_read_command();
            break;
        case SESCREVA:
            handle_write_command();
            break;
        // Nested block command: <comandos>
        case SINICIO:
            handle_commands();
            break;
        default:
            // Error: token does not start a valid command.
            print_sintax_error(NULL);
            break;
    }
}

// <atribuição_chprocedimento>::= (<comando atribuicao>| <chamada de procedimento>)
void handle_assignment_chprocedure() {
    Token aux = current_token;
    get_next_token();
    if (current_token.symbol == SATRIBUICAO) {
        // It's an assignment command.
        get_next_token();
        // Semantic check: type compatibility in assignment.
        if (handle_expression() != get_symbol_from_lexem(aux.lexem)->type) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: tipos incompatíveis na atribuição\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }
    else {
        // It's a procedure call.
        handle_procedure_call(aux);
    }
}

// <comando leitura>::= leia ( <identificador> )
void handle_read_command() {
    get_next_token();
    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            // Semantic action: Check if 'identificador' is an integer variable.
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            if (node != NULL) {
                if (node->type == INT && node->scope == false) {
                    get_next_token();
                    if (current_token.symbol == SFECHA_PARENTESES) {
                        get_next_token();
                    } else {
                        print_sintax_error(")");
                    }
                }
                else {
                    // Semantic error: Not an integer variable.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Semantic error: Symbol does not exist.
                fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não existe\n", current_line, current_token.lexem);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador dentro do comando 'leia'\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        print_sintax_error("(");
    }
}

// <comando escrita>::= escreva ( <identificador> )
void handle_write_command() {
    get_next_token();

    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            // Semantic action: Check if 'identificador' is an integer variable.
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            if (node != NULL) {
                if (node->type == INT && node->scope == false) {
                    get_next_token();
                    if (current_token.symbol == SFECHA_PARENTESES) {
                        get_next_token();
                    } else {
                        print_sintax_error(")");
                    }
                }
                else {
                    // Semantic error: Not an integer variable.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Semantic error: Symbol does not exist.
                fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não existe\n", current_line, current_token.lexem);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador dentro do comando 'escreva'\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        print_sintax_error("(");
    }
}

// <comando enquanto>::= enquanto <expressão> faca <comando>
void handle_while_command() {
    get_next_token();
    handle_expression(); // The expression should evaluate to BOOL
    if (current_token.symbol == SFACA) {
        get_next_token();
        handle_command();
    } else {
        print_sintax_error("faca");
    }
}

// <comando condicional>::= se <expressão> entao <comando> [senao <comando>]
void handle_conditional_command() {
    get_next_token();
    handle_expression(); // The expression should evaluate to BOOL
    if (current_token.symbol == SENTAO) {
        get_next_token();
        handle_command();
        if (current_token.symbol == SSENAO) {
            get_next_token();
            handle_command();
        }
    } else {
        print_sintax_error("entao");
    }
}

// <etapa de declaração de sub-rotinas> ::= (<declaração de procedimento>;|
//                                          <declaração de função>;)
//                                         {<declaração de procedimento>;|
//                                          <declaração de função>;}
void handle_subroutine_section() {
    // Loop to handle multiple procedure or function declarations.
    while (current_token.symbol == SPROCEDIMENTO || current_token.symbol == SFUNCAO) {
        if (current_token.symbol == SPROCEDIMENTO)
            handle_procedure_declaration();
        else
            handle_function_declaration();

        if (current_token.symbol == SPONTO_VIRGULA)
            get_next_token();
        else
            print_sintax_error(";");
    }
}

// <declaração de procedimento> ::= procedimento <identificador>;
//                                 <bloco>
void handle_procedure_declaration() {
    get_next_token();

    if (current_token.symbol == SIDENTIFICADOR) {
        // Semantic action: Check if subroutine can be declared and insert into table.
        if (can_declare_subroutine(current_token.lexem)) {
            insert_node_table(current_token.lexem, true, VOID, -1);

            // CODE GENERATION
            // ...

            get_next_token();
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block();
            } else {
                print_sintax_error(";");
            }
        }
        else {
            // Semantic error: Identifier already exists.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s já existe\n", current_line, current_token.lexem);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador do procedimento após 'procedimento'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    // Semantic action: Pop the current scope.
    pop_scope();
}

// <declaração de função> ::= funcao <identificador>: <tipo>;
//                            <bloco>
void handle_function_declaration() {
    get_next_token();
    if (current_token.symbol == SIDENTIFICADOR) {
        // Semantic action: Check if subroutine can be declared and insert into table.
        if (can_declare_subroutine(current_token.lexem)) {
            insert_node_table(current_token.lexem, true, FUNC, -1);
            
            get_next_token();
            if (current_token.symbol == SDOISPONTOS) {
                get_next_token();
                if (current_token.symbol == SINTEIRO || current_token.symbol == SBOOLEANO) {
                    // Semantic action: Assign return type to the function.
                    if (current_token.symbol == SINTEIRO)
                        insert_type(INT);
                    else
                        insert_type(BOOL);
                    
                    get_next_token();
                    if (current_token.symbol == SPONTO_VIRGULA) {
                        get_next_token();
                        handle_block();
                    } else {
                        print_sintax_error(";");
                    }
                } else {
                    fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado tipo de retorno 'inteiro' ou 'booleano' na declaração da função\n", current_line);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            } else {
                print_sintax_error(":");
            }
        }   
        else {
            // Semantic error: Identifier already exists.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: %s já existe\n", current_line, current_token.lexem);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador da função após 'funcao'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    // Semantic action: Pop the current scope.
    pop_scope();
}

// <expressão>::= <expressão simples> [<operador relacional><expressão simples>]
Type handle_expression() {
    Type type1 = handle_simple_expression();
    // Check for relational operator.
    switch (current_token.symbol) {
        case SMAIOR:
        case SMAIORIG:
        case SIG:
        case SMENOR:
        case SMENORIG:
        case SDIF:
            // Semantic check: Only integer types are allowed for relational operations.
            if (type1 != INT) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            get_next_token();
            Type type2 = handle_simple_expression();
            if (type2 != INT) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            // Relational expression returns boolean type.
            return BOOL;
            break;
        default:
            return type1;
            break;
    }
}

// <expressão simples> ::= [ + | - ] <termo> {( + | - | ou) <termo> }
Type handle_simple_expression() {
    Type ref = VOID;

    // Check for optional unary sign (+ or -).
    if (current_token.symbol == SMAIS || current_token.symbol == SMENOS) {
        get_next_token();
        ref = INT; // Unary signs imply integer type.
    }

    Type type = handle_term();

    // Type check for the first term.
    if (type == INT) {
        ref = INT;
    } else {
        if (ref == INT) {
            // Unary sign with non-integer term is a semantic error.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        } else {
            ref = BOOL; // If no unary sign, the type is determined by the term (BOOL).
        }
    }

    // Loop for additive/OR operations.
    while (current_token.symbol == SMAIS || current_token.symbol == SMENOS || current_token.symbol == SOU) {
        // Semantic check: 'ou' operator with integer type is an error.
        if (current_token.symbol == SOU && ref == INT) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        // Semantic check: Additive operators (+, -) with boolean type is an error.
        else if ((current_token.symbol == SMAIS || current_token.symbol == SMENOS) && ref == BOOL) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        get_next_token();
        // Semantic check: subsequent term must match the expression type.
        if (handle_term() != ref) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }

    return ref; // Return the type of the simple expression.
}

// <termo>::= <fator> {(* | div | e) <fator>}
Type handle_term() {
    Type type1 = handle_factor();
    Simbolo s = current_token.symbol;

    // Loop for multiplicative/AND operations.
    while(current_token.symbol == SMULT || current_token.symbol == SDIV || current_token.symbol == SE) {
        get_next_token();
        Type type2 = handle_factor();
        
        // Semantic check: Multiplication/Division must use INT types.
        if (s == SMULT || s == SDIV) {
            if (type1 != INT || type2 != INT) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação numérica com booleano\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else
                return INT; // Result is INT.
        }
        // Semantic check: 'e' (AND) must use BOOL types.
        else {
            if (type1 != BOOL || type2 != BOOL) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação lógica com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else
                return BOOL; // Result is BOOL.
        }
    }

    return type1; // Return the type of the factor.
}

// <fator> ::= (<variável> |
//              <número> |
//              <chamada de função> |
//              (<expressão>) | verdadeiro | falso |
//              nao <fator>)
Type handle_factor() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            // Semantic action: Check if symbol is a variable or function.
            {
                SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
                if (node != NULL) {
                    if (node->scope == true) { // Subroutine (function or procedure)
                        handle_function_call();
                    } 
                    else { // Variable
                        handle_variable();
                    }
                    return node->type; // Return variable or function return type.
                }
                else {
                    // Semantic error: Symbol does not exist.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: %s não existe\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case SNUMERO:
            get_next_token();
            return INT;
        case SVERDADEIRO:
        case SFALSO:
            get_next_token();
            return BOOL;
        case SNAO: // NOT operator
            get_next_token();
            // Semantic check: 'nao' must be used with BOOL type.
            if (handle_factor() == BOOL) {
                return BOOL;
            }
            else {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operador \"n\" não usado com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        case SABRE_PARENTESES:
            get_next_token();
            Type tipo = handle_expression();

            if (current_token.symbol != SFECHA_PARENTESES) {
                print_sintax_error(")");
            }
            get_next_token();
            return tipo;
        default:
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador, número, 'verdadeiro', 'falso', 'nao' ou '('\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
    }
}

//<comando atribuicao>::= <identificador> := <expressão>
void handle_assignment_command() {
    // This function is not called directly; its logic is inside handle_assignment_chprocedure.
}

// <chamada de procedimento>::= <identificador>
void handle_procedure_call(Token aux) {
    SymbolNode * node = get_symbol_from_lexem(aux.lexem);
    if (node != NULL) {
        // CODE GENERATION
        // ...
    }
    else {
        // Semantic error: Identifier does not exist.
        fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s não existe\n", current_line, aux.lexem);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

// <variável> ::= <identificador>
void handle_variable() {
    // CODE GENERATION (load variable value)
    get_next_token();
}

// <chamada de função> ::= <identificador>
void handle_function_call() {
    // CODE GENERATION (call function)
    get_next_token();
}