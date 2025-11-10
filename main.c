#include <stdlib.h>
#include <stdio.h>
#include <ctype.h> 

#define MAX 100

/* PILHA 1 - operadores */
char stack1[MAX];
int top1 = -1;

void push(char c) {
    stack1[++top1] = c;
}

char pop() {
    return stack1[top1--];
}

char peek() {
    return stack1[top1];
}

int empty() {
    return top1 == -1;
}

int compare_operands(char c) {
    if (c == '*' || c == '/') 
        return 2;
    if (c == '+' || c == '-') 
        return 1;

    return 0;
}

int M[1000];
int s = -1;

void LDC(int k) {
    M[++s] = k;
}

void LDV(int n) {
    M[++s] = M[n];
}

void ADD() {
    M[s-1] = M[s-1] + M[s];
    s--;
}

void SUB() {
    M[s-1] = M[s-1] - M[s];
    s--;
}

void MULT() {
    M[s-1] = M[s-1] * M[s];
    s--;
}

void DIVI() {
    M[s-1] = M[s-1] / M[s];
    s--;
}

void AND() {
    if (M[s-1] == 1 && M[s] == 1){
        M[s-1] = 1;
    } else {
        M[s-1] = 0;
    }
    s--;
}

void OR() {
    if (M[s-1] == 1 || M[s] == 1){
        M[s-1] = 1;
    } else {
        M[s-1] = 0;
    }
    s--;
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
    s--;
}

void CMA() {
    if (M[s-1] > M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }
    s--;
}

void CEQ() {
    if (M[s-1] == M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }
    s--;
}

void CDIF() {
    if (M[s-1] != M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }
    s--;
}

void CMEQ() {
    if (M[s-1] <= M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }
    s--;
}

void CMAQ() {
    if (M[s-1] >= M[s]) {
        M[s-1] = 1; 
    } else {
        M[s-1] = 0;
    }
    s--;
}

void show_stack(){
    if (s == -1) {
        printf("(vazia)\n");
        return;
    }

    for (int i = s; i >= 0; i--) {
        if (i == s)
            printf("| %d |  <-- topo\n", M[i]);
        else
            printf("| %d |\n", M[i]);
    }
}

int main() {
    char string[] = "(2 + 1) * 4";
    char output[MAX];
    int j = 0;

    for (int i = 0; string[i] != '\0'; i++) {
        char c = string[i];

        if (c == ' ')
            continue;

        /*se for digito, vai direto pro output*/
        if (isdigit(c)) {
            output[j++] = c;
        }

        /*se for digito, vai direto pro output*/
        else if (c == '(') {
            push(c);
        }

        /*se for fecha_parenteses, desempilha até abre_parenteses*/
        else if (c == ')') {
            while (!empty() && peek() != '(')
                output[j++] = pop();
            pop();
        }

        /*se for operando, vê se tem outro operando mais forte na pilha*/
        else { 
            while (!empty() && compare_operands(peek()) >= compare_operands(c))
                output[j++] = pop();
            push(c);
        }
    }

    while (!empty())
        output[j++] = pop();

    output[j] = '\0';

    printf("POSFIXA = %s\n\n", output);

    for (int i = 0; output[i] != '\0'; i++) {
        char c = output[i];
        //printf("%c", c);
        if (isdigit(c)) {
            c = c - '0';
            printf("LDC %d\n", c);
            LDC(c);
        }

        else if (isalpha(c)) {
            printf("LDV %c\n", c);
            LDV(c);
        }

        else if (c == '+') {
            printf("ADD\n");
            ADD();
        }

        else if (c == '-') {
            printf("SUB\n");
            SUB();
        }

        else if (c == '*') {
            printf("MULT\n");
            MULT();
        }

        else if (c == '/') {
            printf("DIVI\n");
            DIVI();
        }
    }

    printf("RESULTADO = %d", M[s]);
    return 0;
}
