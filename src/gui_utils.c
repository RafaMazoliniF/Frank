// src/gui_utils.c
#include "includes.h"
#include <stdarg.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// Desfaz a macro definida em includes.h apenas para este arquivo de implementação
// Isso permite chamar a função real exit() ou _exit() do sistema aqui dentro.
#undef exit

jmp_buf env_buffer;
char gui_error_msg[4096];
int gui_mode = 0;

void my_exit_handler(int status) {
    if (gui_mode) {
        longjmp(env_buffer, 1);
    } else {
        // Chama a função real de saída do sistema
        #ifdef _WIN32
            exit(status); 
        #else
            _exit(status);
        #endif
    }
}

int my_fprintf_handler(FILE *stream, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    if (gui_mode && stream == stderr) {
        size_t len = strlen(gui_error_msg);
        
        if (len < sizeof(gui_error_msg) - 100) {
            vsnprintf(gui_error_msg + len, sizeof(gui_error_msg) - len, format, args);
        }
    } else {
        vfprintf(stream, format, args);
    }
    
    va_end(args);
    return 0;
}