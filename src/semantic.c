#include "semantic.h"
#include <string.h>

// ---------------------- Symbol Table ------------------------------
// It is a stack implemented with linked lists where each node is a row of the table

// Creates a new symbol of symbol table
SymbolNode * new_symbol_node(char * lexem, bool scope, Type type, unsigned int mem) {
    SymbolNode * node = (SymbolNode*)malloc(sizeof(SymbolNode));
    node->lexem = lexem;
    node->scope = scope;
    node->type = type;
    node->mem = mem;
    node->next = NULL;

    return node;
}

// Push a new symbol to the table
// receives the node and the pointer to the top of the stack
void push_symbol_node(SymbolNode * node, SymbolNode ** table) {
    node->next = *table;
    *table = node;
}

// Pop a new symbol to the table, removing the top node
// receives the pointer to the top of the stack
// returns the node popped
SymbolNode * pop_symbol_node(SymbolNode ** table) {
    SymbolNode * ret = *table;
    *table = (*table)->next;

    return ret;
}

// ------------------- Semantic ----------------------

// A varible can be declarated if there's no other visible equal identifier at the same scope
bool can_declare_variable(char * lexem, SymbolNode ** symbol_table) {
    SymbolNode * current = *symbol_table;
    bool exited_scope = false;

    while (current->next != NULL && !exited_scope) {
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

// A subroutine can be declarated if there's no other visible equal identifier declareted
bool can_declare_subroutine(char * lexem, SymbolNode ** symbol_table) {
    SymbolNode * current = *symbol_table;

    while (current->next != NULL) {
        if (strcmp(current->lexem, lexem) == 0){
            return false;
        }

        current = current ->next;
    }
    
    return true;
}

bool are_symbols_compatible(SymbolNode * a, SymbolNode * b) {
    if (a->type != b->type) {
        return false;
    }

    return true;
}

