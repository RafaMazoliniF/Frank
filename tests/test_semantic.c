#include "unity.h"
#include "semantic.h" 
#include "sintatic.h"
#include <stdbool.h>

void test_sem1() {
    getFile("tests/semantic_programs/sem7.txt");
    handle_program();
    fclose(file);
}