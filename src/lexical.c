/**
 * @file lexical.c
 * @brief Implementação do Analisador Léxico.
 * * Responsável pela leitura do arquivo fonte, identificação e categorização de tokens,
 * e tratamento de caracteres como espaços, quebras de linha e comentários.
 */
#include "lexical.h"

/** Ponteiro global para o arquivo fonte a ser analisado. */
FILE *file = NULL;
/** Estrutura que armazena o token atualmente reconhecido. */
Token current_token = {0};
/** Contador da linha atual no arquivo fonte para relatórios de erro. */
int current_line = 1;

/**
 * @brief Abre o arquivo fonte especificado para leitura.
 * @param filename O caminho do arquivo a ser aberto.
 */
void getFile(char *filename) {
    file = fopen(filename, "r");  

    if (file == NULL) {
        exit(EXIT_FAILURE);  
    }
}

/**
 * @brief Verifica se uma palavra é um identificador válido (começa com letra ou '_', contém alfanuméricos ou '_').
 * * @param word String a ser validada.
 * @return bool Retorna true se a palavra for um identificador válido, false caso contrário.
 */
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
 * @brief Identifica o símbolo (Simbolo) correspondente a uma palavra ou token.
 * * Compara a string com as palavras reservadas. Caso não seja palavra reservada,
 * classifica como número ou identificador.
 * * @param word String contendo a palavra/token a ser analisada.
 * @return int Valor inteiro correspondente ao enum Simbolo, ou -1 se houver erro.
 */
int get_symbol(char * word){
    if (word == NULL) {
        return -1;
    }

    // Identificação de Palavras Reservadas
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
    
    // Identificação de Operadores e Símbolos
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

    // Identificação de Números e Identificadores (não palavras reservadas)
    if (isdigit(word[0])) {
        // Verifica se é puramente numérico
        for (int i = 1; i < (int)strlen(word); i++) {
            if (!isdigit(word[i])) {
                return SIDENTIFICADOR; // Contém caracteres não numéricos
            }
        }
        return SNUMERO;
    }
    
    // Se não é palavra reservada nem começa com dígito, é um identificador.
    return SIDENTIFICADOR;
}


/**
 * @brief Captura o próximo caracter do arquivo, tratando a contagem de linhas e caracteres de controle.
 * * Ignora o caractere de Retorno de Carro ('\r') e incrementa a contagem de linhas em '\n'.
 * * @return char O próximo caractere lido ou '\0' se for o fim do arquivo (EOF).
 */
char get_next_char() {
    int c;
    
    do {
        c = fgetc(file);
    } while (c == '\r'); 

    if (c == '\n') {
        current_line++;
    }

    if (c == EOF) return '\0'; 
    return (char)c;
}

/**
 * @brief Expande o buffer de caracteres se a capacidade atual for insuficiente.
 * * @param buffer Ponteiro para o buffer atual.
 * @param capacity Ponteiro para a capacidade atual do buffer.
 * @param length Comprimento atual dos dados no buffer.
 * @return char* Ponteiro para o buffer (potencialmente realocado e expandido).
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
 * @brief Captura a próxima "palavra" ou token do arquivo, agrupando caracteres.
 * * Ignora espaços em branco e agrupa:
 * - Sequências de caracteres alfanuméricos ou '_' (identificadores/palavras reservadas).
 * - Sequências de dígitos (números).
 * - Operadores compostos (:=, >=, <=, !=) ou símbolos simples.
 * * @return char* A string (lexema) da próxima palavra ou token alocada dinamicamente, ou NULL se for ENDFILE.
 */
char * get_next_word() {
    int capacity = 16;     
    int length = 0;        
    char *buffer = malloc(capacity);
    int ch;

    if (!buffer) {
        exit(EXIT_FAILURE);
    }

    // Pula espaços em branco
    while ((ch = get_next_char()) != '\0' && isspace(ch));

    if (ch == '\0') {
        free(buffer);
        return NULL;
    }

    // Identifica palavras e identificadores (começam com letra ou '_')
    if (isalpha(ch) || ch == '_') {
        do {
            buffer = ensure_buffer_capacity(buffer, &capacity, length);
            buffer[length++] = ch;
            ch = get_next_char();
        } while (ch != '\0' && (isalnum(ch) || ch == '_'));

        if (ch != '\0') ungetc(ch, file);
    }
    // Identifica números (começam com dígito)
    else if (isdigit(ch)) {
        do {
            buffer = ensure_buffer_capacity(buffer, &capacity, length);
            buffer[length++] = ch;
            ch = get_next_char();
        } while (ch != '\0' && isdigit(ch));

        if (ch != '\0') ungetc(ch, file);
    }

    // Identifica operadores e símbolos
    else {
        buffer = ensure_buffer_capacity(buffer, &capacity, length);
        buffer[length++] = ch;
    
        char next_ch = get_next_char();
        if (next_ch != '\0') {
            // Verifica operadores compostos
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

/**
 * @brief Processa e ignora o conteúdo de um comentário (delimitado por '{' e '}').
 * * Continua lendo palavras até encontrar o delimitador de fechamento '}'.
 * Em caso de fim de arquivo antes do fechamento, um erro é impresso.
 */
void handle_comment() {
    char * next_word;
    do {
        next_word = get_next_word();
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

/**
 * @brief Obtém e armazena o próximo token válido na variável global `current_token`.
 * * Chama `get_next_word` para obter o lexema e `get_symbol` para classificar o token.
 * Trata o caractere de abertura de comentário '{' recursivamente.
 */
void get_next_token() {
    Token token;

    char *word = get_next_word();

    if (word == NULL) {
        token.lexem = NULL;
        token.symbol = ENDFILE;
        current_token = token;
        return;
    }

    // Se for o início de um comentário, trata o comentário e obtém o próximo token.
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