#include "sintatic.h"

// <atribuição_chprocedimento>::= (<comando atribuicao>| <chamada de procedimento>)
void assignment_or_procedure_call() {
    if (current_token.symbol != SIDENTIFICADOR) {
        error("Expected identifier");
    }
    
    get_next_token();
    
    if (current_token.symbol == SATRIBUICAO) {
        assignment_command();
    } else {
        procedure_call();
    }
}

// <comando atribuicao>::= <identificador> := <expressão>
void assignment_command() {
    if (current_token.symbol != SIDENTIFICADOR) {
        error("Expected identifier");
    }
    get_next_token();
    
    if (current_token.symbol != SATRIBUICAO) {
        error("Expected ':='");
    }
    get_next_token();
    
    expression();
}

// <chamada de procedimento>::= <identificador>
void procedure_call() {
    if (current_token.symbol != SIDENTIFICADOR) {
        error("Expected identifier");
    }
    get_next_token();
}

// <comando condicional>::= se <expressão> entao <comando> [senao <comando>]
void conditional_command() {
    if (current_token.symbol != SSE) {
        error("Expected 'if'");
    }
    get_next_token();
    
    expression();
    
    if (current_token.symbol != SENTAO) {
        error("Expected 'then'");
    }
    get_next_token();
    
    command();
    
    if (current_token.symbol == SSENAO) {
        get_next_token();
        command();
    }
}

// <comando enquanto>::= enquanto <expressão> faca <comando>
void while_command() {
    if (current_token.symbol != SENQUANTO) {
        error("Expected 'while'");
    }
    get_next_token();
    
    expression();
    
    if (current_token.symbol != SFACA) {
        error("Expected 'do'");
    }
    get_next_token();
    
    command();
}

// <comando leitura>::= leia ( <identificador> )
void read_command() {
    if (current_token.symbol != SLEIA) {
        error("Expected 'read'");
    }
    get_next_token();
    
    if (current_token.symbol != SABRE_PARENTESES) {
        error("Expected '('");
    }
    get_next_token();
    
    if (current_token.symbol != SIDENTIFICADOR) {
        error("Expected identifier");
    }
    get_next_token();
    
    if (current_token.symbol != SFECHA_PARENTESES) {
        error("Expected ')'");
    }
    get_next_token();
}

// <comando escrita>::= escreva ( <identificador> )
void write_command() {
    if (current_token.symbol != SESCREVA) {
        error("Expected 'write'");
    }
    get_next_token();
    
    if (current_token.symbol != SABRE_PARENTESES) {
        error("Expected '('");
    }
    get_next_token();
    
    if (current_token.symbol != SIDENTIFICADOR) {
        error("Expected identifier");
    }
    get_next_token();
    
    if (current_token.symbol != SFECHA_PARENTESES) {
        error("Expected ')'");
    }
    get_next_token();
}

// <expressão>::= <expressão simples> [<operador relacional><expressão simples>]
void expression() {
    simple_expression();
    
    if (current_token.symbol == SDIF ||
        current_token.symbol == SIG ||
        current_token.symbol == SMENOR ||
        current_token.symbol == SMENORIG ||
        current_token.symbol == SMAIOR ||
        current_token.symbol == SMAIORIG) {
        
        relational_operator();
        simple_expression();
    }
}

// <operador relacional>::= (!= | = | < | <= | > | >=)
void relational_operator() {
    if (current_token.symbol == SDIF ||
        current_token.symbol == SIG ||
        current_token.symbol == SMENOR ||
        current_token.symbol == SMENORIG ||
        current_token.symbol == SMAIOR ||
        current_token.symbol == SMAIORIG) {
        
        get_next_token();
    } else {
        error("Expected relational operator(!= | = | < | <= | > | >=)");
    }
}