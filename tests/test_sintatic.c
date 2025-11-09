#include "lexical.h"
#include "unity.h"
#include "includes.h"
#include "sintatic.h"
#include <stdio.h>

void test_sint1() {
    getFile("tests/sintatic_programs/sint1.txt");
    handle_program();
    fclose(file);
}