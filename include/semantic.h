#include "sintatic.h"

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
    SymbolNode * next;
} SymbolNode;

SymbolNode * new_symbol_node(char * lexem, bool scope, Type type, unsigned int mem);
void push_symbol_node(SymbolNode * node, SymbolNode * table);
SymbolNode * pop_symbol_node(SymbolNode * table);