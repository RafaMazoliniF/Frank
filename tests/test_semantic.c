#include "unity.h"
#include "semantic.h" 
#include <stdbool.h>

void test_can_declare_variable() {
    init_table();

    SymbolNode * var1 = new_symbol_node("var1", false, INT_VAR, 0);
    SymbolNode * func1 = new_symbol_node("func1", true, INT_VAR, 1);
    SymbolNode * var2 = new_symbol_node("var2", false, INT_VAR, 2);
    SymbolNode * var3 = new_symbol_node("var3", false, BOOL_VAR, 3);
    SymbolNode * func2 = new_symbol_node("func2", true, INT_VAR, 5);
    SymbolNode * var4 = new_symbol_node("var4", false, BOOL_VAR, 6);

    push_symbol_node(var1);
    push_symbol_node(func1);
    push_symbol_node(var2);
    push_symbol_node(var3);
    push_symbol_node(func2);
    push_symbol_node(var4);
    

    TEST_ASSERT_TRUE(can_declare_variable("var5"));
    TEST_ASSERT_TRUE(can_declare_variable("var3"));
    TEST_ASSERT_TRUE(can_declare_variable("var2"));
    TEST_ASSERT_FALSE(can_declare_variable("func2"));
    TEST_ASSERT_TRUE(can_declare_variable("func1"));
    TEST_ASSERT_FALSE(can_declare_variable("var4"));
}

void test_can_declare_subroutine() {
    init_table();

    SymbolNode * var1 = new_symbol_node("var1", false, INT_VAR, 0);
    SymbolNode * func1 = new_symbol_node("func1", true, INT_VAR, 1);
    SymbolNode * var2 = new_symbol_node("var2", false, INT_VAR, 2);
    SymbolNode * var3 = new_symbol_node("var3", false, BOOL_VAR, 3);
    SymbolNode * func2 = new_symbol_node("func2", true, INT_VAR, 5);
    SymbolNode * var4 = new_symbol_node("var4", false, BOOL_VAR, 6);

    push_symbol_node(var1);
    push_symbol_node(func1);
    push_symbol_node(var2);
    push_symbol_node(var3);
    push_symbol_node(func2);
    push_symbol_node(var4);
    

    TEST_ASSERT_TRUE(can_declare_subroutine("var5"));
    TEST_ASSERT_FALSE(can_declare_subroutine("var3"));
    TEST_ASSERT_FALSE(can_declare_subroutine("var2"));
    TEST_ASSERT_FALSE(can_declare_subroutine("func2"));
    TEST_ASSERT_FALSE(can_declare_subroutine("func1"));
    TEST_ASSERT_FALSE(can_declare_subroutine("var4"));
    TEST_ASSERT_TRUE(can_declare_subroutine("func10"));
}

void test_are_symbols_compatible() {
    SymbolNode * var1 = new_symbol_node("var1", false, INT_VAR, 1);
    SymbolNode * var2 = new_symbol_node("var2", false, BOOL_VAR, 2);
    SymbolNode * func1 = new_symbol_node("func1", true, INT_VAR, 3);
    SymbolNode * func2 = new_symbol_node("func2", true, BOOL_VAR, 4);

    TEST_ASSERT_FALSE(are_symbols_compatible(var1, var2));
    TEST_ASSERT_TRUE(are_symbols_compatible(var1, func1));
    TEST_ASSERT_FALSE(are_symbols_compatible(var1, func2));
    TEST_ASSERT_FALSE(are_symbols_compatible(func1, func2));
    TEST_ASSERT_TRUE(are_symbols_compatible(var2, func2));
}