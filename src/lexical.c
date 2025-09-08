#include "lexical.h"

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

    return SIDENTIFICADOR;
}


/**
 * @brief Salva o token em um arquivo auxiliar
 * 
 * Salva um token em um arquivo auxiliar no formato <lexema> <simbolo>
 * 
 * @param lexeme lexema a ser identificado e armazenado
 * @param filename arquivo auxiliar que armazena os tokens
 * @return true se sucesso, false caso houve falha
 */
bool save_token(char * lexeme, char * filename) {

    FILE *file = fopen(filename, "a");

    if (file == NULL) {
        printf("Erro: não foi possível abrir o arquivo %s\n", filename);
        exit(EXIT_FAILURE);  
    }

    if (lexeme == NULL || file == NULL) {
        return false;
    }
    
    int symbol = get_symbol(lexeme);
    int result = fprintf(file, "%s %d\n", lexeme, symbol);
    
    fflush(file);
    fclose(file);
    
    if (result < 0) {
        printf("Erro ao escrever no arquivo\n");
        return false;
    }
    
    return true;
}
