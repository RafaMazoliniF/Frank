#include "sintatic.h"

int main() {
    getFile("test.txt");
    handler();

    fclose(file);

    return 0;
}