#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define WORD_MAX_LEN 40

typedef enum {
    SPROGRAMA,          // programa
    SINICIO,            // inicio
    SFIM,               // fim
    SPROCEDIMENTO,      // procedimento
    SFUNCAO,            // funcao
    SSE,                // se
    SENTAO,             // entao
    SSENAO,             // senao
    SENQUANTO,          // enquanto
    SFACA,              // faca
    SESCREVA,           // escreva
    SLEIA,              // leia
    SVAR,               // var
    SINTEIRO,           // inteiro
    SBOOLEANO,          // booleano
    SVERDADEIRO,        // verdadeiro
    SFALSO,             // falso
    SIDENTIFICADOR,     // identificador
    SNUMERO,            // numero
    SATRIBUICAO,        // :=
    SPONTO,             // .
    SPONTO_VIRGULA,     // ;
    SVIRGULA,           // ,
    SABRE_PARENTESES,   // (
    SFECHA_PARENTESES,  // )
    SDOISPONTOS,        // :
    SMAIOR,             // >
    SMAIORIG,           // >=
    SIG,                // =
    SMENOR,             // <
    SMENORIG,           // <=
    SDIF,               // !=
    SMAIS,              // +
    SMENOS,             // -
    SMULT,              // *
    SDIV,               // div
    SE,                 // e
    SOU,                // ou
    SNAO                // nao
} Simbolo;

typedef struct {
    char* lexeme;
    Simbolo symbol;
} Token;

FILE *getFile(char *filename);
char get_next_char(FILE * file);
char * get_next_word(FILE * file);
void handle_comment(FILE * file);
bool isWordValid(const char *word);
int get_symbol(char * word);
//bool save_token(char * lexeme, char * filename);
Token get_next_token(FILE *filename);