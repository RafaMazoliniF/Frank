#include "unity.h"
#include "semantic.h" 

void test_new_symbol_node(void) {
    SymbolNode * new = new_symbol_node("var", false, VAR, 100);
    TEST_ASSERT_EQUAL_STRING(new->lexem, "var");
    TEST_ASSERT_EQUAL(new->scope, false);
    TEST_ASSERT_EQUAL_INT(new->type, 0);
    TEST_ASSERT_EQUAL_UINT(new->mem, 100);
    TEST_ASSERT_EQUAL(new->next, NULL);
}

void test_push_symbol_node(void) {
    init_table();
    SymbolNode * new = new_symbol_node("var", false, VAR, 100);
    push_symbol_node(new);

    TEST_ASSERT_EQUAL(table, new);
    TEST_ASSERT_EQUAL(table->next, NULL);

    SymbolNode * new2 = new_symbol_node("var", true, VAR, 2);
    push_symbol_node(new2);

    TEST_ASSERT_EQUAL(table, new2);
    TEST_ASSERT_EQUAL(table->next, new);
}

void test_pop_symbol_node(void) {
    init_table();
    SymbolNode * new = new_symbol_node("var", false, VAR, 100);
    SymbolNode * new2 = new_symbol_node("var", true, VAR, 2);
    push_symbol_node(new);
    push_symbol_node(new2);

    SymbolNode * ret = pop_symbol_node();

    TEST_ASSERT_EQUAL(table, new);
    TEST_ASSERT_EQUAL(table->next, NULL);
    TEST_ASSERT_EQUAL(ret, new2);
}
