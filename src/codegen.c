#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int label = 0;
int addr = 1; // Keeps 0 to function return values
FILE * gen_file = NULL;

void int_to_string(char ** string, int val) {
    if (val >= 0) {
        sprintf(*string, "%d", val);
    } else {
        sprintf(*string, "    ");
    }
}

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
    
    int_to_string(&slabel, label);
    int_to_string(&sparam1, param1);
    int_to_string(&sparam2, param2);

    if (strcmp(mnemonic, "ALLOC") == 0 || strcmp(mnemonic, "DALLOC") == 0) {
        sprintf(line, "%s %s %s,%s\n", slabel, mnemonic, sparam1, sparam2);
    } else {
        sprintf(line, "%s %s %s %s\n", slabel, mnemonic, sparam1, sparam2);
    }

    if (fputs(line, gen_file) == EOF) {
        perror("falaha ao escrever em result.obj");
        fclose(gen_file);
        exit(EXIT_FAILURE);
    }

    fclose(gen_file);
}