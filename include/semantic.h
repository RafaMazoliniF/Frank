
#ifndef SEMANTIC_H
#define SEMANTIC_H

#pragma once
#include "includes.h"

typedef enum {
    VAR,
    SUBROUTINE,
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
void push_symbol_node(SymbolNode * node, SymbolNode ** table);
SymbolNode * pop_symbol_node(SymbolNode ** table);

bool can_declare_variable(char * lexem, SymbolNode ** symbol_table);
bool can_declare_subroutine(char * lexem, SymbolNode ** symbol_table);
bool are_symbols_compatible(SymbolNode * a, SymbolNode * b);

#endif