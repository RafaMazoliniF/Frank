#ifndef LEXICAL_H
#define LEXICAL_H

#include "includes.h"

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
    SNAO,                // nao
    ENDFILE
} Simbolo;

typedef struct Token {
    Simbolo symbol;
    char * lexem;
} Token;

extern FILE * file;

void getFile(char *filename);
char get_next_char();
char * get_next_word();
void handle_comment();
bool isWordValid(const char *word);
int get_symbol(char * word);
void get_next_token();

extern Token current_token;
extern int current_line;

#endif