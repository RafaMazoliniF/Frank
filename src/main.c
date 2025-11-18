#include "sintatic.h"

int main() {
    getFile("tests/semantic_programs/sem0.txt");
    
    handle_program();

    fclose(file);

    return 0;
}