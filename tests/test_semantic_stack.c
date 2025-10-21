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
    // Se pa que tem que mudar o codigo :(
}


// O 'runner' que executa todos os testes
int main(void) {
    UNITY_BEGIN(); // Inicia o Unity

    RUN_TEST(test_new_symbol_node);

    return UNITY_END(); // Termina o Unity e retorna o resultado
}
