#include "lexical.h"

FILE *file = NULL;
Token current_token = {0};
int current_line = 1;

void getFile(char *filename) {
    file = fopen(filename, "r");  

    if (file == NULL) {
        exit(EXIT_FAILURE);  
    }
}

bool isWordValid(const char *word) {
    if (word == NULL || !isalpha(word[0])) {
        return false;
    }

    for (int i = 1; word[i] != '\0'; i++) {
        if (!(isalpha(word[i]) || isdigit(word[i]) || word[i] == '_')) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Identifica o símbolo correspondente a uma palavra ou token.
 * 
 * Compara a string com as palavras reservadas para determinar o tipo de token correspondente.
 * 
 * @param word String contendo a palavra/token a ser analisada
 * @return int Valor inteiro correspondente ao enum Simbolo, ou -1 se houver erro
 */
int get_symbol(char * word){
    if (word == NULL) {
        return -1;
    }

    if (strcmp(word, "programa") == 0) return SPROGRAMA;
    if (strcmp(word, "inicio") == 0) return SINICIO;
    if (strcmp(word, "fim") == 0) return SFIM;
    if (strcmp(word, "procedimento") == 0) return SPROCEDIMENTO;
    if (strcmp(word, "funcao") == 0) return SFUNCAO;
    if (strcmp(word, "se") == 0) return SSE;
    if (strcmp(word, "entao") == 0) return SENTAO;
    if (strcmp(word, "senao") == 0) return SSENAO;
    if (strcmp(word, "enquanto") == 0) return SENQUANTO;
    if (strcmp(word, "faca") == 0) return SFACA;
    if (strcmp(word, "escreva") == 0) return SESCREVA;
    if (strcmp(word, "leia") == 0) return SLEIA;
    if (strcmp(word, "var") == 0) return SVAR;
    if (strcmp(word, "inteiro") == 0) return SINTEIRO;
    if (strcmp(word, "booleano") == 0) return SBOOLEANO;
    if (strcmp(word, "verdadeiro") == 0) return SVERDADEIRO;
    if (strcmp(word, "falso") == 0) return SFALSO;
    if (strcmp(word, ":=") == 0) return SATRIBUICAO;
    if (strcmp(word, ">=") == 0) return SMAIORIG;
    if (strcmp(word, "<=") == 0) return SMENORIG;
    if (strcmp(word, "!=") == 0) return SDIF;
    if (strcmp(word, ".") == 0) return SPONTO;
    if (strcmp(word, ";") == 0) return SPONTO_VIRGULA;
    if (strcmp(word, ",") == 0) return SVIRGULA;
    if (strcmp(word, "(") == 0) return SABRE_PARENTESES;
    if (strcmp(word, ")") == 0) return SFECHA_PARENTESES;
    if (strcmp(word, ":") == 0) return SDOISPONTOS;
    if (strcmp(word, ">") == 0) return SMAIOR;
    if (strcmp(word, "=") == 0) return SIG;
    if (strcmp(word, "<") == 0) return SMENOR;
    if (strcmp(word, "+") == 0) return SMAIS;
    if (strcmp(word, "-") == 0) return SMENOS;
    if (strcmp(word, "*") == 0) return SMULT;
    if (strcmp(word, "div") == 0) return SDIV;
    if (strcmp(word, "e") == 0) return SE;
    if (strcmp(word, "ou") == 0) return SOU;
    if (strcmp(word, "nao") == 0) return SNAO;

    if (isdigit(word[0])) {
        for (int i = 1; i < (int)strlen(word); i++) {
            if (!isdigit(word[i])) {
                return SIDENTIFICADOR;
            }
        }
    } else {
        return SIDENTIFICADOR;
    }

    return SNUMERO;
}



/**
 * @brief captura o próximo caracter de um dado arquivo
 * 
 * @param file arquivo com permissão de leitura
 * @return ponteiro para o caracter, NULL se houve falha
 */
char get_next_char() {
    int c = fgetc(file);
    if (c == '\n') {
        current_line++;
    }
    if (c == EOF) return '\0'; 
    return (char)c;
}

/**
 * @brief Expande o buffer se necessário
 * 
 * @param buffer ponteiro para o buffer atual
 * @param capacity ponteiro para a capacidade atual
 * @param length comprimento atual do buffer
 * @return char* ponteiro para o buffer expandido
 */
char* ensure_buffer_capacity(char* buffer, int* capacity, int length) {
    if (length + 1 >= *capacity) {
        *capacity *= 2;
        buffer = realloc(buffer, *capacity);
        if (!buffer) {
            exit(EXIT_FAILURE);
        }
    }
    return buffer;
}

/**
 * @brief captura a próxima palavra de um dado arquivo
 * 
 * captura a próxima palavra de um dado arquivo ignorando comentários, espaços ou caracteres não alfanuméricos
 * 
 * @param file arquivo com permissão de leitura
 * @return ponteiro para o caracter, NULL se houve falha
 */
char * get_next_word() {
    int capacity = 16;     
    int length = 0;        
    char *buffer = malloc(capacity);
    char ch;

    if (!buffer) {
        exit(EXIT_FAILURE);
    }

    // Pula espaços em branco
    while ((ch = get_next_char(file)) != '\0' && isspace(ch));

    if (ch == '\0') {
        free(buffer);
        return NULL;
    }

    // Identifica palavras e identificadores
    if (isalpha(ch) || ch == '_') {
        do {
            buffer = ensure_buffer_capacity(buffer, &capacity, length);
            buffer[length++] = ch;
            ch = get_next_char(file);
        } while (ch != '\0' && (isalnum(ch) || ch == '_'));

        if (ch != '\0') ungetc(ch, file);
    }
    // Identifica números
    else if (isdigit(ch)) {
        do {
            buffer = ensure_buffer_capacity(buffer, &capacity, length);
            buffer[length++] = ch;
            ch = get_next_char(file);
        } while (ch != '\0' && isdigit(ch));

        if (ch != '\0') ungetc(ch, file);
    }

    else {
        buffer = ensure_buffer_capacity(buffer, &capacity, length);
        buffer[length++] = ch;
    
        char next_ch = get_next_char(file);
        if (next_ch != '\0') {
            if ((ch == ':' && next_ch == '=') ||  // :=
                (ch == '>' && next_ch == '=') ||  // >=
                (ch == '<' && next_ch == '=') ||  // <=
                (ch == '!' && next_ch == '=')) {  // !=
                
                buffer = ensure_buffer_capacity(buffer, &capacity, length);
                buffer[length++] = next_ch;
            } else {
                ungetc(next_ch, file);
            }
        }
    }

    buffer[length] = '\0';
    return buffer;
}

void handle_comment() {
    char * next_word;
    do {
        next_word = get_next_word(file);
        if (next_word == NULL) {
            printf("Erro: comentário não fechado\n");
            break;
        }
        
        if (strcmp("}", next_word) == 0) {
            free(next_word);
            return;
        }
        
        free(next_word);
    } while (next_word != NULL);
}

void get_next_token() {
    Token token;

    char *word = get_next_word(file);

    if (word == NULL) {
        token.lexem = NULL;
        token.symbol = ENDFILE;
        current_token = token;
        return;
    }

    if (strcmp(word, "{") == 0) {
        free(word);
        handle_comment();
        get_next_token(); 
        return;
    }

    token.lexem = word;
    token.symbol = get_symbol(word);

    current_token = token;
}