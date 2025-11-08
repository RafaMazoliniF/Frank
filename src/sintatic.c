#include "sintatic.h"
#include "lexical.h"
#include "semantic.h"
#include <stdbool.h>
#include <stdio.h>

void print_error(const char *expected, const char *context) {
    fprintf(stderr, "\n=== ERRO SINTÁTICO ===\n");
    fprintf(stderr, "Contexto: %s\n", context);
    fprintf(stderr, "Esperado: %s\n", expected);
    fprintf(stderr, "Encontrado: ");
    
    if (current_token.lexem != NULL) {
        fprintf(stderr, "'%s' (símbolo: %d)\n", current_token.lexem, current_token.symbol);
    } else {
        fprintf(stderr, "EOF (fim de arquivo)\n");
    }
    
    fprintf(stderr, "=====================\n\n");

    fflush(stderr);
    exit(EXIT_FAILURE);
}

// Macro para simplificar chamadas de erro
#define ERROR(expected, context) print_error(expected, context)

void handler() {
    get_next_token();
    handle_program();
}

// <programa>::= programa <identificador> ; <bloco> .
void handle_program() {
    if (current_token.symbol == SPROGRAMA) {
        get_next_token();

        if (current_token.symbol == SIDENTIFICADOR) {

            insert_node_table(current_token.lexem, true, PROGRAM_NAME, 1);

            get_next_token();

            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();

                handle_block();

                if (current_token.symbol == SPONTO) {
                    get_next_token();
                    
                    if (current_token.lexem != NULL) {
                        print_error("EOF", "após fim de programa");
                    }

                } else {
                    print_error("'.'", "após bloco do programa");
                }

            } else {
                print_error("';'", "após identificador do programa");
            }

        } else {
            print_error("identificador", "após 'programa'");
        }

    } else {
        print_error("'programa'", "início do arquivo");
    }
}

// <bloco>::= [<etapa de declaração de variáveis>]
//            [<etapa de declaração de sub-rotinas>]
//            <comandos>
void handle_block() {
    handle_variable_declaration_section();
    handle_subroutine_section();
    handle_commands();
}

// <etapa de declaração de variáveis>::= var <declaração de variáveis> ;
//                                           {<declaração de variáveis>;}
void handle_variable_declaration_section(){
    if (current_token.symbol == SVAR) {
    
        get_next_token();

        if (current_token.symbol == SIDENTIFICADOR) {
            while (current_token.symbol == SIDENTIFICADOR) {
                handle_variables(); 

                if (current_token.symbol == SPONTO_VIRGULA) {
                    get_next_token();
                } else {
                    print_error("';'", "após declaração de variáveis");
                }
            }

        } else {
            print_error("identificador", "após 'var'");
        }
    }
}

// <declaração de variáveis>::= <identificador> {, <identificador>} : <tipo>
void handle_variables() {
    if (current_token.symbol != SIDENTIFICADOR || !can_declare_variable(current_token.lexem)) {
        print_error("identificador", "declaração de variáveis");
    }

    while (current_token.symbol == SIDENTIFICADOR) {
        if (!can_declare_variable(current_token.lexem)) {
            print_error("identificador ja existe", "semantic: declaração de variáveis");
        }

        insert_node_table(current_token.lexem, false, VAR, -1);

        get_next_token();

        if (current_token.symbol == SVIRGULA) {
        
            get_next_token();

            if (current_token.symbol == SDOISPONTOS) {
                print_error("identificador", "após ',' em declaração de variáveis");
            }

        } else if (current_token.symbol == SDOISPONTOS) {
            break; 
        } else {
            print_error("',' ou ':'", "em declaração de variáveis");
        }
    }

    get_next_token();
    handle_type();
}

// <tipo> ::= (inteiro | booleano)
void handle_type() {
    if (current_token.symbol == SINTEIRO || current_token.symbol == SBOOLEANO) {
        if (current_token.symbol == SINTEIRO) {
            insert_type(INT);
        } else if (current_token.symbol == SBOOLEANO) {
            insert_type(BOOL);
        }

        get_next_token();
    } else {
        print_error("'inteiro' ou 'booleano'", "tipo de variável");
    }
}

// <etapa de declaração de sub-rotinas> ::= (<declaração de procedimento>;|
//                                           <declaração de função>;)
//                                           {<declaração de procedimento>;|
//                                           <declaração de função>;}
void handle_subroutine_section() {
    while (current_token.symbol == SPROCEDIMENTO || current_token.symbol == SFUNCAO) {
        if (current_token.symbol == SPROCEDIMENTO) {
            handle_procedure_declaration();  
        } else if (current_token.symbol == SFUNCAO) {
            handle_function_declaration();
        }

        if (current_token.symbol == SPONTO_VIRGULA) {
            get_next_token();
        } else {
            print_error("';'", "após declaração de sub-rotina");
        }
    }
}

// <declaração de procedimento> ::= procedimento <identificador>;
//                                               <bloco>
void handle_procedure_declaration() {
    if (current_token.symbol == SPROCEDIMENTO) {
    
        get_next_token();
    } else {
        print_error("'procedimento'", "declaração de procedimento");
    }

    if (current_token.symbol == SIDENTIFICADOR && can_declare_subroutine(current_token.lexem)) {
        get_next_token();
    } else {
        print_error("identificador válido", "após 'procedimento'");
    }

    if (current_token.symbol == SPONTO_VIRGULA) {
    
        get_next_token();
    } else {
        print_error("';'", "após nome do procedimento");
    }

    handle_block();
}

// <declaração de função> ::= funcao <identificador>: <tipo>;
//                                 <bloco>
void handle_function_declaration() {
    if (current_token.symbol == SFUNCAO) {
    
        get_next_token();
    } else {
        print_error("'funcao'", "declaração de função");
    }

    if (current_token.symbol == SIDENTIFICADOR && can_declare_subroutine(current_token.lexem)) {
        get_next_token();
    } else {
        print_error("identificador válido", "após 'funcao'");
    }

    if (current_token.symbol == SDOISPONTOS) {
    
        get_next_token();
    } else {
        print_error("':'", "após nome da função");
    }

    handle_type();

    if (current_token.symbol == SPONTO_VIRGULA) {
    
        get_next_token();
    } else {
        print_error("';'", "após tipo de retorno da função");
    }

    handle_block();
}

// <comandos>::= inicio
//                  <comando>{;<comando>}[;]
//               fim
void handle_commands() {
    if (current_token.symbol == SINICIO) {
        get_next_token();
    } else {
        print_error("'inicio'", "bloco de comandos");
    }

    handle_command();

    while (current_token.symbol == SPONTO_VIRGULA) {
    
        get_next_token();

        if (current_token.symbol == SFIM) {
            break;
        }
        handle_command();
    }

    if (current_token.symbol == SFIM) {
    
        get_next_token();
    } else {
        print_error("'fim'", "fechamento do bloco de comandos");
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

        case SINICIO: 
            handle_commands();
            break;
        case SFIM:
            break;
        default:
            print_error("identificador, 'se', 'enquanto', 'leia', 'escreva' ou 'inicio'", "comando");
    }
}

// <atribuição_chprocedimento>::= (<comando atribuicao>| <chamada de procedimento>)
void handle_assignment_chprocedure() {
    if (current_token.symbol == SIDENTIFICADOR) {
        SymbolNode * node = get_symbol_from_lexem(current_token.lexem);

        if (node == NULL) {
            print_error("identificador não existe", "atribuição ou chamada de procedimento");
        }

        // If identifier is a procedure
        if (node->type == VOID) {
            handle_procedure_call();
            get_next_token();
        } else {
            handle_assignment_command();
            get_next_token();
        }
    } 
    
    else {
        print_error("identificador", "atribuição ou chamada de procedimento");
    }
}

//<comando atribuicao>::= <identificador> := <expressão>
void handle_assignment_command() {
    if (current_token.symbol != SIDENTIFICADOR) {
        print_error("identificador", "comando de atribuição");
    }

    get_next_token();

    if (current_token.symbol != SATRIBUICAO) {
        print_error("atribuidor", "comando de atribuição");
    }

    get_next_token();

    handle_expression();
}

// <chamada de procedimento>::= <identificador>
void handle_procedure_call() {
    if (current_token.symbol != SIDENTIFICADOR) {
        print_error("identificador", "chamada de procedimento");
    }

    // ------------------ Semantic -----------------
    // Verify if procedure exists
    SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
    if (node == NULL) {
        print_error("identificador existente", "chamada de procedimento");
    }

    else if (node->type != VOID) {
        print_error("procedimento", "chamada de procedimento");
    }
    // --------------------------------------------

    get_next_token();
}

// <comando condicional>::= se <expressão> entao <comando> [senao <comando>]
void handle_conditional_command() {
    if (current_token.symbol != SSE) {
        print_error("'se'", "comando condicional");
    }
    get_next_token();
    
    handle_expression();
    
    if (current_token.symbol != SENTAO) {
        print_error("'entao'", "após expressão do 'se'");
    }
    get_next_token();
    
    handle_command();
    
    if (current_token.symbol == SSENAO) {
        get_next_token();
        handle_command();
    }
}

// <comando enquanto>::= enquanto <expressão> faca <comando>
void handle_while_command() {
    if (current_token.symbol != SENQUANTO) {
        print_error("'enquanto'", "comando de repetição");
    }
    get_next_token();
    
    handle_expression();
    
    if (current_token.symbol != SFACA) {
        print_error("'faca'", "após expressão do 'enquanto'");
    }
    get_next_token();
    
    handle_command();
}

// <comando leitura>::= leia ( <identificador> )
void handle_read_command() {
    if (current_token.symbol != SLEIA) {
        print_error("'leia'", "comando de leitura");
    }
    get_next_token();
    
    if (current_token.symbol != SABRE_PARENTESES) {
        print_error("'('", "após 'leia'");
    }
    get_next_token();  
    
    if (current_token.symbol != SIDENTIFICADOR) {
        print_error("identificador", "dentro de 'leia(...)'");
    }

    // ---------------Semantic----------------
    // verify if identifier exists
    SymbolNode * symbol_node = get_symbol_from_lexem(current_token.lexem);
    if (symbol_node == NULL) {
        print_error("", "identificador nao existe");
    } 
    
    // and if variable is a INT
    else if (symbol_node->type != INT && symbol_node->scope == true) {
        print_error("var inteiro", "escreva");
    }
    // --------------------------------------

    get_next_token();
    
    if (current_token.symbol != SFECHA_PARENTESES) {
        print_error("')'", "fechamento de 'leia'");
    }
    get_next_token();
}

// <comando escrita>::= escreva ( <identificador> )
void handle_write_command() {
    if (current_token.symbol != SESCREVA) {
        print_error("'escreva'", "comando de escrita");
    }
    get_next_token();
    
    if (current_token.symbol != SABRE_PARENTESES) {
        print_error("'('", "após 'escreva'");
    }
    get_next_token();
    
    if (current_token.symbol != SIDENTIFICADOR) {
        print_error("identificador", "dentro de 'escreva(...)'");
    }

    // ---------------Semantic----------------
    // verify if identifier exists
    SymbolNode * symbol_node = get_symbol_from_lexem(current_token.lexem);
    if (symbol_node == NULL) {
        print_error("", "identificador nao existe");
    } 
    
    // and if variable is a INT
    else if (symbol_node->type != INT && symbol_node->scope == true) {
        print_error("var inteiro", "escreva");
    }
    // --------------------------------------
    
    get_next_token();
    
    if (current_token.symbol != SFECHA_PARENTESES) {
        print_error("')'", "fechamento de 'escreva'");
    }
    get_next_token();
}

// <expressão>::= <expressão simples> [<operador relacional><expressão simples>]
void handle_expression() {
    handle_simple_expression();
    
    if (current_token.symbol == SDIF ||
        current_token.symbol == SIG ||
        current_token.symbol == SMENOR ||
        current_token.symbol == SMENORIG ||
        current_token.symbol == SMAIOR ||
        current_token.symbol == SMAIORIG) {
        
        get_next_token();
        handle_simple_expression();
    }
}

// <expressão simples> ::= [ + | - ] <termo> {( + | - | ou) <termo> }
void handle_simple_expression() {
    if (current_token.symbol == SMAIS || current_token.symbol == SMENOS) {
        get_next_token();
    }
    
    handle_term();

    while(current_token.symbol == SMAIS || current_token.symbol == SMENOS || current_token.symbol == SOU) {
        get_next_token();
        handle_term();
    }   
}

// <termo>::= <fator> {(* | div | e) <fator>}
void handle_term() {
    handle_factor();

    while(current_token.symbol == SMULT || current_token.symbol == SDIV || current_token.symbol == SE) {
        get_next_token();
        handle_factor();
    }
}

// <fator> ::= (<variável> |
//              <número> |
//              <chamada de função> |
//              (<expressão>) | verdadeiro | falso |
//              nao <fator>)
void handle_factor() {

    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            {SymbolNode * node = get_symbol_from_lexem(current_token.lexem);

            if (node != NULL) {
                switch (node->type) {
                    case INT_VAR:
                    case BOOL_VAR:
                        handle_variable();
                        break;
                    case INT_FUNC:
                    case BOOL_FUNC:
                        handle_function_call();
                        break;
                    default:
                        print_error("tipo válido", "fator");
                }
            }}

            break;
        case SNUMERO:
        case SVERDADEIRO:
        case SFALSO:
            get_next_token();
            break;
        case SNAO:
            get_next_token();
            handle_factor();
            break;
        case SABRE_PARENTESES:
            get_next_token();
            handle_expression();

            if (current_token.symbol != SFECHA_PARENTESES) {
                print_error("')'", "fechamento de expressão entre parênteses");
            }

            get_next_token();
            break;
        default:
            print_error("identificador, número, 'verdadeiro', 'falso', 'nao' ou '('", "fator em expressão");
    }
}

// <variável> ::= <identificador>
void handle_variable() {
    if (current_token.symbol != SIDENTIFICADOR) {
        print_error("identificador", "variável");
    }
}