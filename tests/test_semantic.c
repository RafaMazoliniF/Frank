#include "unity.h"
#include "semantic.h" 
#include <stdbool.h>

void test_can_declare_variable() {
    SymbolNode * table = NULL;

    SymbolNode * var1 = new_symbol_node("var1", false, INT, 0);
    SymbolNode * func1 = new_symbol_node("func1", true, INT, 1);
    SymbolNode * var2 = new_symbol_node("var2", false, INT, 2);
    SymbolNode * var3 = new_symbol_node("var3", false, BOOL, 3);
    SymbolNode * func2 = new_symbol_node("func2", true, INT, 5);
    SymbolNode * var4 = new_symbol_node("var4", false, BOOL, 6);

    push_symbol_node(var1, &table);
    push_symbol_node(func1, &table);
    push_symbol_node(var2, &table);
    push_symbol_node(var3, &table);
    push_symbol_node(func2, &table);
    push_symbol_node(var4, &table);
    

    TEST_ASSERT_TRUE(can_declare_variable("var5", &table));
    TEST_ASSERT_TRUE(can_declare_variable("var3", &table));
    TEST_ASSERT_TRUE(can_declare_variable("var2", &table));
    TEST_ASSERT_FALSE(can_declare_variable("func2", &table));
    TEST_ASSERT_TRUE(can_declare_variable("func1", &table));
    TEST_ASSERT_FALSE(can_declare_variable("var4", &table));
}

void test_can_declare_subroutine() {
    SymbolNode * table = NULL;

    SymbolNode * var1 = new_symbol_node("var1", false, INT, 0);
    SymbolNode * func1 = new_symbol_node("func1", true, INT, 1);
    SymbolNode * var2 = new_symbol_node("var2", false, INT, 2);
    SymbolNode * var3 = new_symbol_node("var3", false, BOOL, 3);
    SymbolNode * func2 = new_symbol_node("func2", true, INT, 5);
    SymbolNode * var4 = new_symbol_node("var4", false, BOOL, 6);

    push_symbol_node(var1, &table);
    push_symbol_node(func1, &table);
    push_symbol_node(var2, &table);
    push_symbol_node(var3, &table);
    push_symbol_node(func2, &table);
    push_symbol_node(var4, &table);
    

    TEST_ASSERT_TRUE(can_declare_subroutine("var5", &table));
    TEST_ASSERT_FALSE(can_declare_subroutine("var3", &table));
    TEST_ASSERT_FALSE(can_declare_subroutine("var2", &table));
    TEST_ASSERT_FALSE(can_declare_subroutine("func2", &table));
    TEST_ASSERT_FALSE(can_declare_subroutine("func1", &table));
    TEST_ASSERT_FALSE(can_declare_subroutine("var4", &table));
    TEST_ASSERT_TRUE(can_declare_subroutine("func10", &table));
}

void test_are_symbols_compatible() {
    SymbolNode * var1 = new_symbol_node("var1", false, INT, 1);
    SymbolNode * var2 = new_symbol_node("var2", false, BOOL, 2);
    SymbolNode * func1 = new_symbol_node("func1", true, INT, 3);
    SymbolNode * func2 = new_symbol_node("func2", true, BOOL, 4);

    TEST_ASSERT_FALSE(are_symbols_compatible(var1, var2));
    TEST_ASSERT_TRUE(are_symbols_compatible(var1, func1));
    TEST_ASSERT_FALSE(are_symbols_compatible(var1, func2));
    TEST_ASSERT_FALSE(are_symbols_compatible(func1, func2));
    TEST_ASSERT_TRUE(are_symbols_compatible(var2, func2));
}