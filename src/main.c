#include "lexical.h"

int main() {
    FILE * file = getFile("test.txt");

    char * word = get_next_word(file);
    while (word != NULL) {
        if (strcmp(word, "{") == 0) {
            handle_comment(file);
            word = get_next_word(file);
        }

        save_token(word, "res.txt");
        word = get_next_word(file);
    }

    return 0;
}