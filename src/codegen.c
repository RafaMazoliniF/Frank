/**
 * @file codegen.c
 * @brief Implementação das funções de Geração de Código.
 * * Este módulo é responsável por gerar as instruções da Máquina Virtual (MVD)
 * e gerenciar contadores de rótulos e endereços de memória.
 */
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Contador global para geração de rótulos (labels) sequenciais. */
int label = 0;
/** Contador de endereço de memória para alocação de variáveis. O endereço 0 é reservado para retorno de função. */
int addr = 1;
/** Ponteiro para o arquivo de saída onde o código objeto (.obj) é escrito. */
FILE * gen_file = NULL;

/**
 * @brief Converte um valor inteiro para sua representação em string, formatada para o código objeto.
 * * Rótulos (labels) recebem um espaçamento extra. Valores negativos ou -1 (para parâmetros não utilizados)
 * são representados por espaços.
 * * @param string Ponteiro para a string de saída (deve ser alocada).
 * @param val O valor inteiro a ser convertido.
 * @param is_label Flag indicando se o valor é um rótulo.
 */
void int_to_string(char ** string, int val, bool is_label) {
    if (val >= 0) {
        if (is_label) {
            sprintf(*string, "%d   ", val);
        } else {
            sprintf(*string, "%d", val);
        }
    } else {
        sprintf(*string, "    ");
    }
}

/**
 * @brief Gera uma linha de instrução da Máquina Virtual (MVD) e escreve no arquivo de saída.
 * * A instrução é formatada com rótulo (opcional), mnemônico e até dois parâmetros.
 * O arquivo de saída "result.obj" é aberto e fechado em cada chamada (modo 'a').
 * * @param label Rótulo da instrução (-1 se não houver).
 * @param mnemonic O mnemônico da instrução (ex: "LDC", "ADD", "JMP").
 * @param param1 Primeiro parâmetro (-1 se não houver).
 * @param param2 Segundo parâmetro (-1 se não houver).
 */
void generate(int label, const char * mnemonic, int param1, int param2) {
    gen_file = fopen("result.obj", "a");
    if (gen_file == NULL) {
        perror("result.obj não abriu");
        exit(EXIT_FAILURE);
    }

    char line[30];

    char * slabel = malloc(10 * sizeof(char));
    char * sparam1 = malloc(10 * sizeof(char));
    char * sparam2 = malloc(10 * sizeof(char));
    
    int_to_string(&slabel, label, true);
    int_to_string(&sparam1, param1, false);
    int_to_string(&sparam2, param2, false);

    
    sprintf(line, "%s %s %s %s\n", slabel, mnemonic, sparam1, sparam2);

    if (fputs(line, gen_file) == EOF) {
        perror("falaha ao escrever em result.obj");
        fclose(gen_file);
        exit(EXIT_FAILURE);
    }

    fclose(gen_file);
}