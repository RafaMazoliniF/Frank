#include "unity.h"
#include "semantic.h" // Ficheiro com o código a ser testado

// As funções setUp e tearDown são executadas antes e depois de cada teste.
// Úteis para inicializar/limpar recursos.
void setUp(void) {
    // Pode deixar em branco se não precisar
}

void tearDown(void) {
    // Pode deixar em branco se não precisar
}

void test_new_symbol_node(void) {
    SymbolNode * new = new_symbol_node("var", false, VAR, 100);
    TEST_ASSERT_EQUAL_STRING(new->lexem, "var");
    TEST_ASSERT_EQUAL(new->scope, false);
    TEST_ASSERT_EQUAL_INT(new->type, 0);
    TEST_ASSERT_EQUAL_UINT(new->mem, 100);
    TEST_ASSERT_EQUAL(new->next, NULL);
}

void test_push_symbol_node(void) {
    SymbolNode * stack = NULL;
    SymbolNode * new = new_symbol_node("var", false, VAR, 100);
    push_symbol_node(new, &stack);

    TEST_ASSERT_EQUAL(stack, new);
    TEST_ASSERT_EQUAL(stack->next, NULL);

    SymbolNode * new2 = new_symbol_node("var", true, VAR, 2);
    push_symbol_node(new2, &stack);

    TEST_ASSERT_EQUAL(stack, new2);
    TEST_ASSERT_EQUAL(stack->next, new);
}

void test_pop_symbol_node(void) {
    SymbolNode * stack = NULL;
    SymbolNode * new = new_symbol_node("var", false, VAR, 100);
    SymbolNode * new2 = new_symbol_node("var", true, VAR, 2);
    push_symbol_node(new, &stack);
    push_symbol_node(new2, &stack);

    SymbolNode * ret = pop_symbol_node(&stack);

    TEST_ASSERT_EQUAL(stack, new);
    TEST_ASSERT_EQUAL(stack->next, NULL);
    TEST_ASSERT_EQUAL(ret, new2);
}


int main(void) {
    UNITY_BEGIN(); // Inicia o Unity

    RUN_TEST(test_new_symbol_node);
    RUN_TEST(test_push_symbol_node);
    RUN_TEST(test_pop_symbol_node);

    return UNITY_END(); // Termina o Unity e retorna o resultado
}
