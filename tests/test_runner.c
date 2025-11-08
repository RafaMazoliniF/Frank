#include "test_semantic_stack.c"
#include "test_semantic.c"
#include "unity_internals.h"
#include "test_sintatic+semantic.c"

void setUp(void) {
    // Pode deixar em branco se não precisar
}

void tearDown(void) {
    // Pode deixar em branco se não precisar
}

int main(void) {
    UNITY_BEGIN(); // Inicia o Unity

    RUN_TEST(test_new_symbol_node);
    RUN_TEST(test_push_symbol_node);
    RUN_TEST(test_pop_symbol_node);

    RUN_TEST(test_can_declare_variable);
    RUN_TEST(test_can_declare_subroutine);
    RUN_TEST(test_are_symbols_compatible);

    //RUN_TEST(test_min);
    //RUN_TEST(test_sucess_declarations);
    //RUN_TEST(test_fail_parenteses_leia);

    return UNITY_END(); // Termina o Unity e retorna o resultado
}