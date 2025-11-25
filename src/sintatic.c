/**
 * @file sintatic.c
 * @brief Implementação do Analisador Sintático (Parser) com ações Semânticas e Geração de Código.
 */
#include "sintatic.h"
#include "codegen.h"
#include "lexical.h"
#include "semantic.h"

// Protótipo auxiliar para evitar implicit declaration warning
void print_sintax_error(const char* expected);

/**
 * @brief Imprime uma mensagem de erro sintático no console de erro (stderr) e encerra a execução.
 */
void print_sintax_error(const char* expected) {
    fprintf(stderr, "ERRO SINTÁTICO na linha %d: ", current_line);
    if (expected != NULL) {
        fprintf(stderr, "esperado '%s', encontrado '%s'\n", expected, current_token.lexem);
    } else {
        fprintf(stderr, "token inesperado '%s'\n", current_token.lexem);
    }
    fflush(stderr);
    exit(EXIT_FAILURE);
}

/**
 * @brief Analisa a estrutura principal do programa: <programa>::= programa <identificador> ; <bloco> .
 */
void handle_program() {
    label = 1;
    generate(-1, "START", -1, -1);
    generate(-1, "ALLOC", 0, 1);

    get_next_token();

    if (current_token.symbol == SPROGRAMA) {
        get_next_token();

        if (current_token.symbol == SIDENTIFICADOR) {
            // Ação Semântica: Insere o nome do programa na tabela de símbolos.
            insert_node_table(current_token.lexem, true, UNDEFINED, PROGRAM_NAME, -1);
            get_next_token();

            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block(); // Chamada para a análise do bloco principal

                if (current_token.symbol == SPONTO) {
                    get_next_token();

                    // Verifica se há código após o ponto final
                    if (current_token.lexem != NULL || current_token.symbol != ENDFILE) {
                        fprintf(stderr, "ERRO SINTÁTICO na linha %d: código após o fim do programa\n", current_line);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    } else {
                        // Geração de Código: Desaloca a área de memória do programa e encerra a execução.
                        generate(-1, "DALLOC", 0, 1);
                        generate(-1, "HLT", -1, -1);
                    }
                } else {
                    print_sintax_error(".");
                }
            } else {
                print_sintax_error(";");
            }
        } else {
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador do programa após 'programa'\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: programa deve começar com a palavra 'programa'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Analisa a estrutura de um bloco
 */
void handle_block() {
    int count = handle_variable_declaration_section();
    int addr_to_dealloc = addr;
    
    // Geração de Código: Aloca espaço para as variáveis declaradas no escopo.
    if (count > 0) {
        generate(-1, "ALLOC", addr, count);
        addr += count;
    }

    int aux_label = label;
    // Analisa a seção de sub-rotinas (procedimentos/funções)
    bool has_inner_subprogram = handle_subroutine_section();

    // Se houver sub-rotinas aninhadas, insere o rótulo de retorno para o corpo do bloco.
    if (has_inner_subprogram) {
        generate(aux_label, "NULL ", -1, -1);
    }

    handle_commands(); // Analisa o corpo de comandos
    
    // Geração de Código: Desaloca o espaço das variáveis locais ao sair do bloco.
    if (count > 0) {
        generate(-1, "DALLOC", addr_to_dealloc, count);
        addr -= count;
    }
}

/**
 * @brief Analisa a seção de declaração de variáveis
 */
int handle_variable_declaration_section(){
    int count = 0;

    if (current_token.symbol == SVAR) {
        get_next_token();

        if (current_token.symbol == SIDENTIFICADOR) {
            while (current_token.symbol == SIDENTIFICADOR) {
                handle_variables(&count);

                if (current_token.symbol == SPONTO_VIRGULA) {
                    get_next_token();
                } else {
                    print_sintax_error(";");
                }
            }
        } else {
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador de variável após 'var'\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }

    return count;
}

void handle_variables(int * count) {
    do {
        if (current_token.symbol == SIDENTIFICADOR) {
            // Ação Semântica: Verifica se a variável pode ser declarada e insere na tabela.
            if (can_declare_variable(current_token.lexem)) {
                insert_node_table(current_token.lexem, false, UNDEFINED, VAR, addr + (*count));
                *count += 1;

                get_next_token();

                if (current_token.symbol == SVIRGULA) {
                    get_next_token();
                    if (current_token.symbol != SIDENTIFICADOR && current_token.symbol != SDOISPONTOS) {
                        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador após ','\n", current_line);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    }
                } else if (current_token.symbol == SDOISPONTOS) {
                    break;
                } else {
                    print_sintax_error(",' ou ':");
                }
            }
            else {
                // Erro Semântico: Variável já existe no escopo.
                fprintf(stderr, "ERRO SEMANTICO na linha %d: variável %s já existe\n", current_line, current_token.lexem);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        } else {
            print_sintax_error("identificador");
        }
    } while (current_token.symbol != SDOISPONTOS);

    get_next_token();
    handle_type(); // Analisa o tipo da declaração
}


/**
 * @brief Analisa o tipo de dado: <tipo> ::= (inteiro | booleano)
 */
void handle_type() {
    if (current_token.symbol != SINTEIRO && current_token.symbol != SBOOLEANO) {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado tipo 'inteiro' ou 'booleano'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    else {
        // Ação Semântica: Atribui o tipo (TYPE_INT ou TYPE_BOOL) aos nós pendentes.
        if (current_token.symbol == SINTEIRO)
            insert_type(TYPE_INT);
        else 
            insert_type(TYPE_BOOL);
    }
    get_next_token();
}

/**
 * @brief Analisa o bloco de comandos
 */
void handle_commands() {
    if (current_token.symbol == SINICIO) {
        get_next_token();
        handle_command();

        while (current_token.symbol != SFIM) {
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                // Permite ponto e vírgula opcional antes do 'fim'
                if (current_token.symbol != SFIM) {
                    handle_command();
                }
            } else {
                print_sintax_error(";");
            }
        }
        // Consome o token 'fim'.
        get_next_token();

    } else {
        print_sintax_error("inicio");
    }
}

/**
 * @brief Analisa um comando genérico
 */
void handle_command() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            handle_assignment_chprocedure(); 
            break;
        case SSE:
            handle_conditional_command(); 
            break;
        case SENQUANTO:
            handle_while_command(); 
            break;
        case SLEIA:
            handle_read_command(); 
            break;
        case SESCREVA:
            handle_write_command(); 
            break;
        case SINICIO:
            handle_commands(); 
            break;
        default:
            print_sintax_error(NULL);
            break;
    }
}

void handle_assignment_chprocedure() {
    Token aux = current_token;
    get_next_token();
    if (current_token.symbol == SATRIBUICAO) {
        handle_assignment_command(aux); 
    }
    else {
        handle_procedure_call(aux); 
    }
}

/**
 * @brief Analisa o comando de leitura: <comando leitura>::= leia ( <identificador> )
 */
void handle_read_command() {
    get_next_token();
    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            
            if (node != NULL) {
                // Checagem Semântica: Verifica se é uma variável inteira
                if (node->data_type == TYPE_INT) {

                    // Geração de Código: Leitura e armazenamento do valor.
                    generate(-1, "RD   ", -1, -1);
                    generate(-1, "STR  ", node->mem, -1);

                    get_next_token();
                    if (current_token.symbol == SFECHA_PARENTESES) {
                        get_next_token();
                    } else {
                        print_sintax_error(")");
                    }
                }
                else {
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não existe\n", current_line, current_token.lexem);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador dentro do comando 'leia'\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        print_sintax_error("(");
    }
}

/**
 * @brief Analisa o comando de escrita
 */
void handle_write_command() {
    get_next_token();

    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            
            if (node != NULL) {
                // Checagem Semântica: Verifica se é uma variável inteira e não um marcador de escopo.
                if (node->data_type == TYPE_INT && node->scope == false) {

                    // Geração de Código: Carrega o valor e imprime.
                    generate(-1, "LDV  ", node->mem, -1);
                    generate(-1, "PRN", -1, -1);

                    get_next_token();
                    if (current_token.symbol == SFECHA_PARENTESES) {
                        get_next_token();
                    } else {
                        print_sintax_error(")");
                    }
                }
                else {
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não existe\n", current_line, current_token.lexem);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador dentro do comando 'escreva'\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        print_sintax_error("(");
    }
}

/**
 * @brief Analisa o comando de repetição 'enquanto'
 */
void handle_while_command() {
    int l1 = label++; // Rótulo para o início do loop (expressão)
    int l2 = label++; // Rótulo para sair do loop

    // Geração de Código: Ponto de entrada do loop
    generate(l1, "NULL ", -1, -1);

    get_next_token();
    // Checagem Semântica: A expressão deve ser booleana.
    if (handle_expression() != TYPE_BOOL) { 
        fprintf(stderr, "ERRO SEMANTICO na linha %d: expressão deve ser BOOL\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    } 

    // Geração de Código: Se a expressão for FALSO (0), salta para o rótulo de saída.
    generate(-1, "LDC  ", 1, -1);
    generate(-1, "CEQ", -1, -1);
    generate(-1, "JMPF ", l2, -1);

    if (current_token.symbol == SFACA) {
        get_next_token();
        handle_command(); // Analisa o corpo do loop
    } else {
        print_sintax_error("faca");
    }

    // Geração de Código: Salta de volta para o início do loop (expressão)
    generate(-1, "JMP  ", l1, -1);
    generate(l2, "NULL ", -1, -1); // Rótulo de saída
}

/**
 * @brief Analisa o comando condicional 'se'
 */
void handle_conditional_command() {
    int l1 = label++; // Rótulo para o bloco 'senao' ou final

    get_next_token();
    // Checagem Semântica: A expressão deve ser booleana.
    if (handle_expression() != TYPE_BOOL) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: expressão deve ser BOOL\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    // Geração de Código: Se a expressão for FALSO (0), salta para o rótulo do 'senao' ou final.
    generate(-1, "LDC  ", 1, -1);
    generate(-1, "CEQ  ", -1, -1);
    generate(-1, "JMPF ", l1, -1);

    if (current_token.symbol == SENTAO) {
        get_next_token();
        handle_command(); // Analisa o bloco 'entao'

        if (current_token.symbol == SSENAO) {
            int l2 = label++; // Rótulo para o final do 'se/senao'

            // Geração de Código: Salta o bloco 'senao'
            generate(-1, "JMP  ", l2, -1);
            generate(l1, "NULL ", -1, -1); // Rótulo de entrada do 'senao'

            get_next_token();
            handle_command(); // Analisa o bloco 'senao'

            generate(l2, "NULL ", -1, -1); // Rótulo final
        } else {
            generate(l1, "NULL ", -1, -1); // Rótulo final (se não houver 'senao')
        }
    } else {
        print_sintax_error("entao");
    }
}

/**
 * @brief Analisa a seção de declaração de sub-rotinas
 */
bool handle_subroutine_section() {
    bool has_inner_subprogram = false;

    if (current_token.symbol == SPROCEDIMENTO || current_token.symbol == SFUNCAO) {
        generate(-1, "JMP  ", label, -1);
        label++;
        has_inner_subprogram = true;
    }

    while (current_token.symbol == SPROCEDIMENTO || current_token.symbol == SFUNCAO) {
        if (current_token.symbol == SPROCEDIMENTO)
            handle_procedure_declaration();
        else
            handle_function_declaration();

        if (current_token.symbol == SPONTO_VIRGULA)
            get_next_token();
        else
            print_sintax_error(";");
    }

    return has_inner_subprogram;
}

void handle_procedure_declaration() {
    int l1 = label++; 
    SymbolNode * proc;

    get_next_token();

    if (current_token.symbol == SIDENTIFICADOR) {
        if (can_declare_subroutine(current_token.lexem)) {
            insert_node_table(current_token.lexem, true, UNDEFINED, PROCEDURE, -1);
            proc = table;

            generate(l1, "NULL", -1, -1); 

            get_next_token();
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block(); 
            } else {
                print_sintax_error(";");
            }
        }
        else {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s já existe\n", current_line, current_token.lexem);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador do procedimento após 'procedimento'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    pop_scope(proc);
    proc->mem = l1; 
    generate(-1, "RETURN", -1, -1); 
}

void handle_function_declaration() {
    int l1 = label++; 
    SymbolNode * func;

    get_next_token();
    if (current_token.symbol == SIDENTIFICADOR) {
        if (can_declare_subroutine(current_token.lexem)) {
            insert_node_table(current_token.lexem, true, UNDEFINED, FUNC, l1);
            func = table;

            generate(l1, "NULL ", -1, -1); 
            
            get_next_token();
            if (current_token.symbol == SDOISPONTOS) {
                get_next_token();
                if (current_token.symbol == SINTEIRO || current_token.symbol == SBOOLEANO) {
                    if (current_token.symbol == SINTEIRO)
                        insert_type(TYPE_INT);
                    else
                        insert_type(TYPE_BOOL);
                    
                    get_next_token();
                    if (current_token.symbol == SPONTO_VIRGULA) {
                        get_next_token();
                        handle_block(); 
                    } else {
                        print_sintax_error(";");
                    }
                } else {
                    fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado tipo de retorno 'inteiro' ou 'booleano' na declaração da função\n", current_line);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            } else {
                print_sintax_error(":");
            }
        }   
        else {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: %s já existe\n", current_line, current_token.lexem);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador da função após 'funcao'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    pop_scope(func);
    func->mem = l1; 
    generate(-1, "RETURN", -1, -1); 
}

void process_relational_operator(DataType * type1) {
    if (*type1 != TYPE_INT) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis1\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    get_next_token();
    DataType type2 = handle_simple_expression();
    if (type2 != TYPE_INT) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis2\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Analisa uma expressão
 */
DataType handle_expression() {
    DataType type1 = handle_simple_expression();
    
    if (current_token.symbol == SMAIOR) {
        process_relational_operator(&type1);
        generate(-1, "CMA  ", -1, -1);
        return TYPE_BOOL;
    }
    if (current_token.symbol == SMAIORIG) {
        process_relational_operator(&type1);
        generate(-1, "CMAQ ", -1, -1);
        return TYPE_BOOL;
    }
    if (current_token.symbol == SIG) {
        process_relational_operator(&type1);
        generate(-1, "CEQ  ", -1, -1);
        return TYPE_BOOL;
    }
    if (current_token.symbol == SMENOR) {
        process_relational_operator(&type1);
        generate(-1, "CME  ", -1, -1);
        return TYPE_BOOL;
    }
    if (current_token.symbol == SMENORIG) {
        process_relational_operator(&type1);
        generate(-1, "CMEQ ", -1, -1);
        return TYPE_BOOL;
    }
    if (current_token.symbol == SDIF) {
        process_relational_operator(&type1);
        generate(-1, "CDIF ", -1, -1);
        return TYPE_BOOL;
    }
    
    return type1; 
}

/**
 * @brief Analisa uma expressão simples
 */
DataType handle_simple_expression() {
    DataType ref = UNDEFINED;

    if (current_token.symbol == SMAIS || current_token.symbol == SMENOS) {
        get_next_token();
        ref = TYPE_INT; // Sinais unários implicam tipo inteiro.
    }

    DataType type = handle_term(); 

    if (type == TYPE_INT) {
        ref = TYPE_INT;
    } else {
        if (ref == TYPE_INT) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis3\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        } else {
            ref = TYPE_BOOL; 
        }
    }

    Simbolo * operations = (Simbolo*)malloc(100 * sizeof(Simbolo));
    int i_op = 0;

    while (current_token.symbol == SMAIS || current_token.symbol == SMENOS || current_token.symbol == SOU) {
        operations[i_op] = current_token.symbol;
        i_op++;

        if (current_token.symbol == SOU && ref == TYPE_INT) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis4\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        else if ((current_token.symbol == SMAIS || current_token.symbol == SMENOS) && ref == TYPE_BOOL) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis5\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        get_next_token();
        if (handle_term() != ref) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis6\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < i_op; i++) {
        switch (operations[i]) {
            case SMAIS:
                generate(-1, "ADD  ", -1, -1);
                break;
            case SMENOS:
                generate(-1, "SUB  ", -1, -1);
                break;
            case SOU:
                generate(-1, "OR   ", -1, -1);
                break;
            default:
                break;
        }
    }

    free(operations);
    return ref; 
}

/**
 * @brief Analisa um termo
 */
DataType handle_term() {
    DataType type1 = handle_factor();
    Simbolo s = current_token.symbol;

    Simbolo * operations = (Simbolo*)malloc(100 * sizeof(Simbolo));
    int i_op = 0;

    DataType type_to_return = -1;

    while(current_token.symbol == SMULT || current_token.symbol == SDIV || current_token.symbol == SE) {
        operations[i_op] = current_token.symbol;
        i_op++;

        get_next_token();
        DataType type2 = handle_factor();
        
        if (s == SMULT || s == SDIV) {
            if (type1 != TYPE_INT || type2 != TYPE_INT) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação numérica com booleano\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else {
                type_to_return = TYPE_INT; 
            }
        }
        else {
            if (type1 != TYPE_BOOL || type2 != TYPE_BOOL) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação lógica com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else {
                type_to_return = TYPE_BOOL; 
            }
        }
        s = current_token.symbol;
    }

    for (int i = 0; i < i_op; i++) {
        switch (operations[i]) {
            case SMULT:
                generate(-1, "MULT ", -1, -1);
                break;
            case SDIV:
                generate(-1, "DIVI ", -1, -1);
                break;
            case SE:
                generate(-1, "AND  ", -1, -1);
                break;
            default:
                break;
        }
    }

    free(operations);

    if (type_to_return == TYPE_INT || type_to_return == TYPE_BOOL) {
        return type_to_return;
    } else {
        return type1; 
    }
}

/**
 * @brief Analisa um fator
 */
DataType handle_factor() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            {
                SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
                
                if (node != NULL) {
                    if (node->structure_type == FUNC) { 
                        handle_function_call(node);
                    } 
                    else if (node->structure_type == VAR) { 
                        handle_variable(node);
                    }
                    else {
                        fprintf(stderr, "ERRO SEMANTICO na linha %d: %s não é uma variável ou função\n", current_line, current_token.lexem);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    }
                    return node->data_type; 
                }
                else {
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: %s não existe\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case SNUMERO:
            generate(-1, "LDC  ", atoi(current_token.lexem), -1);
            get_next_token();
            return TYPE_INT;
        case SVERDADEIRO:
            generate(-1, "LDC  ", 1, -1);
            get_next_token();
            return TYPE_BOOL;
        case SFALSO:
            generate(-1, "LDC  ", 0, -1);
            get_next_token();
            return TYPE_BOOL;
        case SNAO: 
            get_next_token();
            if (handle_factor() == TYPE_BOOL) {
                generate(-1, "NEG  ", -1, -1); 
                return TYPE_BOOL;
            }
            else {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operador \"n\" não usado com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        case SABRE_PARENTESES:
            get_next_token();
            DataType tipo = handle_expression(); 

            if (current_token.symbol != SFECHA_PARENTESES) {
                print_sintax_error(")");
            }
            get_next_token();
            return tipo;
        default:
            fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador, número, 'verdadeiro', 'falso', 'nao' ou '('\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
    }
    return UNDEFINED;
}

/**
 * @brief Analisa o comando de atribuição
 */
void handle_assignment_command(Token aux) {
    get_next_token();

    SymbolNode * obj_to_assign_value = get_symbol_from_lexem(aux.lexem);

    DataType t1 = handle_expression();
    DataType t2 = obj_to_assign_value->data_type;

    if (t1 != t2) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: tipos incompatíveis na atribuição\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    } 

    if (obj_to_assign_value->structure_type == FUNC) {
        generate(-1, "STR  ", 0, -1);
    } else {
        generate(-1, "STR  ", obj_to_assign_value->mem, -1);
    }
}

void handle_procedure_call(Token aux) {
    SymbolNode * node = get_symbol_from_lexem(aux.lexem);
    if (node != NULL) {
        generate(-1, "CALL ", node->mem, -1);
    }
    else {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s não existe\n", current_line, aux.lexem);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

void handle_variable(SymbolNode * node) {
    generate(-1, "LDV  ", node->mem, -1);
    get_next_token();
}

void handle_function_call(SymbolNode * node) {
    generate(-1, "CALL ", node->mem, -1);
    generate(-1, "LDV  ", 0, -1);
    get_next_token();
}