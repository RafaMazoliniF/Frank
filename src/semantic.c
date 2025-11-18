#include "semantic.h"
#include <string.h>

// ---------------------- Symbol Table ------------------------------
// It is a stack implemented with linked lists where each node is a row of the table

SymbolNode * table = NULL;

void init_table() {
    table = NULL;
}

// Creates a new symbol of symbol table
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

// Push a new symbol to the table
// receives the node and the pointer to the top of the stack
void push_symbol_node(SymbolNode * node) {
    node->next = table;
    table = node;
}

void insert_node_table(char * lexem, bool scope, DataType data_type, StructureType structure_type, unsigned int mem) {
    SymbolNode * node = new_symbol_node(lexem, scope, data_type, structure_type, mem);
    push_symbol_node(node);
}

// Pop a new symbol to the table, removing the top node
// receives the pointer to the top of the stack
// returns the node popped
SymbolNode * pop_symbol_node() {
    SymbolNode * ret = table;
    table = table->next;

    return ret;
}

// ------------------- Semantic ----------------------

// A varible can be declarated if there's no other visible equal identifier at the same scope
bool can_declare_variable(char * lexem) {
    SymbolNode * current = table;
    bool exited_scope = false;

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

// A subroutine can be declarated if there's no other visible equal identifier declareted
bool can_declare_subroutine(char * lexem) {
    SymbolNode * current = table;

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
    for (SymbolNode * current = table; current != NULL; current = current->next) {
        printf("%s; ", current->lexem);
    }

    printf("\n\n");

    while (table != final && table != NULL) {
        pop_symbol_node();
    }

    for (SymbolNode * current = table; current != NULL; current = current->next) {
        printf("%s; ", current->lexem);
    }

    printf("\n\n");
}

