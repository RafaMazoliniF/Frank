#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "includes.h"

typedef enum {
    VAR,
    INT,
    BOOL
} Type;

typedef struct SymbolNode {
    char * lexem;
    bool scope;
    Type type;
    unsigned int mem;
    struct SymbolNode * next;
} SymbolNode;

SymbolNode * new_symbol_node(char * lexem, bool scope, Type type, unsigned int mem);
void push_symbol_node(SymbolNode * node, SymbolNode * table);
SymbolNode * pop_symbol_node(SymbolNode * table);

#endif