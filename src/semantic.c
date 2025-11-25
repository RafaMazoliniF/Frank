/**
 * @file semantic.c
 * @brief Implementação das funcionalidades de gerenciamento da Tabela de Símbolos e Análise Semântica.
 * * A Tabela de Símbolos é implementada como uma pilha de nós (SymbolNode) para suportar escopo aninhado.
 */
#include "semantic.h"
#include <string.h>

// ---------------------- Tabela de Símbolos ------------------------------
// É uma pilha implementada com listas encadeadas onde cada nó é uma linha da tabela.

/** Ponteiro para o topo da Tabela de Símbolos (pilha). */
SymbolNode * table = NULL;

/**
 * @brief Inicializa a Tabela de Símbolos, definindo o topo como NULL.
 */
void init_table() {
    table = NULL;
}

/**
 * @brief Cria e inicializa um novo nó para a Tabela de Símbolos.
 */
SymbolNode * new_symbol_node(char * lexem, bool scope, DataType data_type, StructureType structure_type, unsigned int mem) {
    SymbolNode * node = (SymbolNode*)malloc(sizeof(SymbolNode));
    node->lexem = lexem;
    node->scope = scope;
    node->data_type = data_type;
    node->structure_type = structure_type;
    node->mem = mem;
    node->next = NULL;

    return node;
}

/**
 * @brief Insere um novo nó no topo da Tabela de Símbolos (operação PUSH).
 */
void push_symbol_node(SymbolNode * node) {
    node->next = table;
    table = node;
}

/**
 * @brief Cria um novo nó e o insere no topo da Tabela de Símbolos.
 */
void insert_node_table(char * lexem, bool scope, DataType data_type, StructureType structure_type, unsigned int mem) {
    SymbolNode * node = new_symbol_node(lexem, scope, data_type, structure_type, mem);
    push_symbol_node(node);
}

/**
 * @brief Remove o nó do topo da Tabela de Símbolos (operação POP).
 */
SymbolNode * pop_symbol_node() {
    SymbolNode * ret = table;
    table = table->next;

    return ret;
}

// ------------------- Análise Semântica ----------------------

bool can_declare_variable(char * lexem) {
    SymbolNode * current = table;
    bool exited_scope = false;

    // Percorre a pilha até o início do escopo (ou o final da pilha)
    while (current != NULL && !exited_scope) {
        if (strcmp(current->lexem, lexem) == 0) {
            return false;
        }

        if (current->scope == true) {
            exited_scope = true;
        }
        current = current->next;
    }

    return true;
}

bool can_declare_subroutine(char * lexem) {
    SymbolNode * current = table;

    // Percorre toda a pilha
    while (current != NULL) {
        if (strcmp(current->lexem, lexem) == 0){
            return false;
        }

        current = current ->next;
    }
    
    return true;
}

SymbolNode * get_symbol_from_lexem(char * lexem) {
    for (SymbolNode * current = table; current != NULL; current = current->next) {
        if (strcmp(current->lexem, lexem) == 0) {
            return current;
        }
    }

    return NULL;
}

/**
 * @brief Atribui um tipo de dado a todos os símbolos recém-declarados (com tipo UNDEFINED).
 */
void insert_type(DataType data_type) {
    for (SymbolNode * current = table; current->structure_type == VAR || current->structure_type == FUNC; current = current->next) {
        if (current->data_type == UNDEFINED) {
            current->data_type = data_type;
        }

        if (current->next == NULL) {
            break;
        }
    }
}

void pop_scope(SymbolNode * final) {
    while (table != final && table != NULL) {
        pop_symbol_node();
    }
}