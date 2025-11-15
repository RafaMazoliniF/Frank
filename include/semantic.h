
#ifndef SEMANTIC_H
#define SEMANTIC_H

#pragma once
#include "includes.h"

typedef enum {
    INT,
    BOOL,
    UNDEFINED
} DataType;

typedef enum {
    PROGRAM_NAME,
    VAR,
    FUNC,
    PROCEDURE,
} StructureType;

typedef struct SymbolNode {
    char * lexem;
    bool scope;
    DataType data_type;
    StructureType structure_type;
    unsigned int mem;
    struct SymbolNode * next;
} SymbolNode;

extern SymbolNode * table;

void init_table();

SymbolNode * new_symbol_node(char * lexem, bool scope, DataType data_type, StructureType structure_type, unsigned int mem);
void push_symbol_node(SymbolNode * node);
void insert_node_table(char * lexem, bool scope, DataType data_type, StructureType structure_type, unsigned int mem);
SymbolNode * pop_symbol_node();

bool can_declare_variable(char * lexem);
bool can_declare_subroutine(char * lexem);
bool are_symbols_compatible(SymbolNode * a, SymbolNode * b);
SymbolNode * get_symbol_from_lexem(char * lexem);
void insert_type(DataType data_type);
void pop_scope();

#endif