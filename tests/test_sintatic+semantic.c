#include "lexical.h"
#include "unity.h"
#include "semantic.h" 
#include "includes.h"
#include "sintatic.h"
#include <stdio.h>

void write_program(const char * program) {
    FILE *f = fopen("tests/test.txt", "w");

    if (f == NULL) {
        perror("Erro ao abrir o arquivo test.txt");
        exit(EXIT_FAILURE); 
    }

    fprintf(f, "%s", program);
    fclose(f);
}

void test_min() {
    const char *program = 
        "programa minimo;\n"
        "inicio\n"
        "    inicio\n"
        "    fim\n"
        "fim.\n";

    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}

void test_sucess_declarations(){
    const char *program = 
        "programa declaracoes;\n"
        "var\n"
        "    x, y, z: inteiro;\n"
        "    flag: booleano;\n\n"
        "procedimento p1;\n"
        "var\n"
        "    local_p: inteiro;\n"
        "inicio\n"
        "    local_p := 1\n"
        "fim;\n\n"
        "funcao f1: booleano;\n"
        "inicio\n"
        "    f1 := verdadeiro\n"
        "fim;\n\n"
        "inicio\n"
        "    x := 10\n"
        "fim.";

    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}

void test_sucess_commands(){
    const char *program = 
        "programa comandos;\n"
        "var\n"
        "    a, b: inteiro;\n"
        "inicio\n"
        "    leia(a);\n"
        "    leia(b);\n"
        "\n"
        "    se (a > b) entao\n"
        "        escreva(a)\n"
        "    senao\n"
        "        escreva(b);\n"
        "\n"
        "    enquanto (a < 10) faca\n"
        "    inicio\n"
        "        a := a + 1;\n"
        "        se (a = 5) entao\n"
        "            escreva(a)\n"
        "    fim\n"
        "fim.";

    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}

void test_fail_ponto_virgula_programa() {
    const char *program = 
        "programa falha_ponto_virgula\n"
        "inicio\n"
        "    inicio\n"
        "    fim\n"
        "fim.";
    
    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}

void test_fail_ponto_final() {
    const char *program = 
        "programa falha_ponto_final;\n"
        "inicio\n"
        "    inicio\n"
        "    fim\n"
        "fim";
    
    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}

void test_fail_var_pos_bloco() {
    const char *program = 
        "programa falha_var_pos_bloco;\n"
        "inicio\n"
        "    inicio\n"
        "    fim\n"
        "fim.\n"
        "\n"
        "var\n"
        "    x: inteiro;";
    
    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}

void test_fail_atribuicao_errada() {
    const char *program = 
        "programa falha_atribuicao;\n"
        "var\n"
        "    x: inteiro;\n"
        "inicio\n"
        "    x = 10\n"
        "fim.";
    
    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}

void test_fail_parenteses_leia() {
    const char *program = 
        "programa falha_parenteses_leia;\n"
        "var\n"
        "    x: inteiro;\n"
        "inicio\n"
        "    leia x)\n"
        "fim.";
    
    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}

void test_fail_faca_faltando() {
    const char *program = 
        "programa falha_faca_faltando;\n"
        "var\n"
        "    x: inteiro;\n"
        "inicio\n"
        "    x := 0;\n"
        "    enquanto (x < 10)\n"
        "    inicio\n"
        "        x := x + 1\n"
        "    fim\n"
        "fim.";
    
    write_program(program);
    getFile("tests/test.txt");

    handler();
    fclose(file);
}