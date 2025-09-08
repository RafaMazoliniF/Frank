#include "lexical.h"

int main() {
    char *filename = "res.txt";

    char * word = "inteiro";
    save_token(word, filename);

    char * word1 = "var";
    save_token(word1, filename);

    char * word2 = ">";
    save_token(word2, filename);

    return 0;
}