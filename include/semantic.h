
#ifndef SEMANTIC_H
#define SEMANTIC_H

#pragma once
#include "includes.h"

typedef enum {
    VOID, // Only procedures are void
    INT, // variables or function return
    BOOL // variables or function return
} Type;

typedef struct SymbolNode {
    char * lexem;
    bool scope;
    Type type;
    unsigned int mem;
    struct SymbolNode * next;
} SymbolNode;

extern SymbolNode * table;

void init_table();

SymbolNode * new_symbol_node(char * lexem, bool scope, Type type, unsigned int mem);
void push_symbol_node(SymbolNode * node);
SymbolNode * pop_symbol_node();

bool can_declare_variable(char * lexem);
bool can_declare_subroutine(char * lexem);
bool are_symbols_compatible(SymbolNode * a, SymbolNode * b);
SymbolNode * get_symbol_from_lexem(char * lexem);

#endif