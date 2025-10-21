#include "semantic.h"

SymbolNode * new_symbol_node(char * lexem, bool scope, Type type, unsigned int mem) {
    SymbolNode * node = (SymbolNode*)malloc(sizeof(SymbolNode));
    node->lexem = lexem;
    node->scope = scope;
    node->type = type;
    node->mem = mem;
    node->next = NULL;

    return node;
}


void push_symbol_node(SymbolNode * node, SymbolNode * table) {
    node->next = table;
    table = node;
}

SymbolNode * pop_symbol_node(SymbolNode * table) {
    SymbolNode * ret = table;
    table = table->next;

    return ret;
}