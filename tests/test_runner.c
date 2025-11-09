// #include "test_semantic_stack.c"
// #include "test_semantic.c"
#include "unity_internals.h"
#include "test_sintatic.c"
#include "test_semantic.c"

void setUp(void) {
    // Pode deixar em branco se não precisar
}

void tearDown(void) {
    // Pode deixar em branco se não precisar
}

int main(void) {
    UNITY_BEGIN(); // Inicia o Unity

    // RUN_TEST(test_new_symbol_node);
    // RUN_TEST(test_push_symbol_node);
    // RUN_TEST(test_pop_symbol_node);

    
    RUN_TEST(test_sem1);
    //RUN_TEST(test_sint1);


    return UNITY_END(); // Termina o Unity e retorna o resultado
}