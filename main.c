#include <stdlib.h>
#include <stdio.h>
#include <ctype.h> 

#define MAX 100

char stack[MAX];
int top = -1;

void push(char c) {
    stack[++top] = c;
}

char pop() {
    return stack[top--];
}

char see_stack() {
    return stack[top];
}

int end_stack() {
    return top == -1;
}

int compare_operands(char c) {
    if (c == '*' || c == '/')
        return 2;

    if (c == '+' || c == '-')
        return 1;

    return 0;
}

/*INSTRUÇÕES*/
int M[1000];
int s = -1;

void LDC(int k) {
    s = s + 1;
    M[s] = k;
}

void LDV(int n) {
    s = s + 1;
    M[s] = M[n];
}

void ADD() {
    M[s-1] = M[s-1] + M[s];
    s = s - 1;
}

void SUB() {
    M[s-1] = M[s-1] - M[s];
    s = s - 1;
}

void MULT() {
    M[s-1] = M[s-1] * M[s];
    s = s - 1;
}

void DIVI() {
    M[s-1] = M[s-1] / M[s];
    s = s - 1; 
}

void AND() {
    if (M[s-1] == 1 && M[s] == 1){
        M[s-1] = 1;
    } else {
        M[s-1] = 0;
    }
    s = s - 1;
}

void OR() {
    if (M[s-1] == 1 || M[s] == 1){
        M[s-1] = 1;
    } else {
        M[s-1] = 0;
    }
    s = s - 1;
}

void NEG() {
    M[s] = 1 - M[s];
}

void CME() {
    if (M[s-1] < M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }

    s = s - 1;
}

void CMA() {
    if (M[s-1] > M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }

    s = s - 1;
}

void CEQ() {
    if (M[s-1] == M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }

    s = s - 1;
}

void CDIF() {
    if (M[s-1] != M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }

    s = s - 1;
}

void CMEQ() {
    if (M[s-1] <= M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }

    s = s - 1;
}

void CMAQ() {
    if (M[s-1] >= M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }

    s = s - 1;
}

int main() {
    char output[MAX];
    char string[] = "2 + 4 * 3";
    int j = 0;

    for (int i = 0; string[i] != '\0'; i++) {
        char c = string[i];

        if (c == ' ')
            continue;
        
        /*vê se é variavel -> vai direto pro output*/
        if (isalnum(c)) {
            output[j++] = c;
        }

        /*vê se é ( -> vai pra pilha*/
        else if (c == '(') {
            push(c);
        }

        /*vê se é ) -> desempilha até ( e desempilha (*/
        else if (c == ')') {
            while (!end_stack() && see_stack() != '(') {
                output[j++] = pop();
            }
            pop(); 
        }

        /*vê se é operando e se for compara se tem um operando maior no topo da pilha*/
        else {
            while (!end_stack() && compare_operands(see_stack()) >= compare_operands(c)) 
            {
                output[j++] = pop();
            }
            push(c);
        }
    }

    /*desempilha tudo no fim*/
    while (!end_stack()) {
        output[j++] = pop();
    }

    output[j] = '\0';

    printf("POSFIXA = %s\n", output);

    /*=============================================*/
    j = 0;

    for (int i = 0; output[i] != '\0'; i++) {
        char c = output[i];

        if (isalnum(c)) {
            //immutable[j++] = c;
            printf("LDC %c\n", c);
        }

        else if(c == '+')
            printf("ADD\n");
        else if(c == '-')
            printf("SUB\n");
        else if(c == '*')
            printf("MULT\n");
        else if(c == '/')
            printf("DIVI\n");
    }

    return 0;
}
