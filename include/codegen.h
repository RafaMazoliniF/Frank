#ifndef CODEGEN_H
#define CODEGEN_H

#include "includes.h"

extern int label;
extern int addr;
extern FILE * gen_file;



void generate(int label, const char * mnemonic, int param1, int param2);

#endif