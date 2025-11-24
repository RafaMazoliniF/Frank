// include/includes.h
#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>

// Variáveis globais para a GUI
extern jmp_buf env_buffer;
extern char gui_error_msg[4096]; // <--- ALTERADO DE 2048 PARA 4096
extern int gui_mode; 

// Substitui o exit()
void my_exit_handler(int status);
#define exit(status) my_exit_handler(status)

// Substitui fprintf(stderr)
#define fprintf(stream, format, ...) my_fprintf_handler(stream, format, ##__VA_ARGS__)
int my_fprintf_handler(FILE *stream, const char *format, ...);

#endif