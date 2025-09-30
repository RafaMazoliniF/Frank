#include "sintatico.h"

int main() {
    FILE *file = getFile("test.txt");

    parser(file);

    fclose(file);

    return 0;
}