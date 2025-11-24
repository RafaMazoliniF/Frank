/**
 * @file sintatic.c
 * @brief Implementação do Analisador Sintático (Parser) com ações Semânticas e Geração de Código.
 * * Este módulo realiza a análise descendente preditiva para verificar a correção sintática
 * do código, executando checagens semânticas e gerando o código de máquina virtual (MVD)
 * em paralelo.
 */
#include "sintatic.h"
#include "codegen.h"
#include "lexical.h"
#include "semantic.h"

/**
 * @brief Imprime uma mensagem de erro sintático no console de erro (stderr) e encerra a execução.
 * * @param expected O lexema esperado para o token atual, ou NULL para um token inesperado genérico.
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
 * @brief Analisa a estrutura de um bloco: <bloco>::= [<etapa de declaração de variáveis>] [<etapa de declaração de sub-rotinas>] <comandos>
 * * Gerencia a alocação e desalocação de memória para variáveis locais.
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
 * @brief Analisa a seção de declaração de variáveis: <etapa de declaração de variáveis>::= var <declaração de variáveis> ; {<declaração de variáveis>;}
 * * @return int O número total de variáveis declaradas.
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

/**
 * @brief Analisa uma declaração de variáveis: <declaração de variáveis>::= <identificador> {, <identificador>} : <tipo>
 * * Realiza a inserção de cada variável na Tabela de Símbolos.
 * * @param count Ponteiro para o contador de variáveis no escopo atual.
 */
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
            // ===================================
        } else {
            print_sintax_error("identificador");
        }
    } while (current_token.symbol != SDOISPONTOS);

    get_next_token();
    handle_type(); // Analisa o tipo da declaração
}


/**
 * @brief Analisa o tipo de dado: <tipo> ::= (inteiro | booleano)
 * * Realiza a ação semântica de atribuição de tipo aos símbolos declarados.
 */
void handle_type() {
    if (current_token.symbol != SINTEIRO && current_token.symbol != SBOOLEANO) {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado tipo 'inteiro' ou 'booleano'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    else {
        // Ação Semântica: Atribui o tipo (INT ou BOOL) aos nós pendentes.
        if (current_token.symbol == SINTEIRO)
            insert_type(INT);
        else 
            insert_type(BOOL);
    }
    get_next_token();
}

/**
 * @brief Analisa o bloco de comandos: <comandos>::= inicio <comando>{;<comando>}[;] fim
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
                // Erro: token não é 'fim' nem ';', indicando falta de separador.
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
 * @brief Analisa um comando genérico, identificando-o pelo primeiro token.
 */
void handle_command() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            handle_assignment_chprocedure(); // Pode ser atribuição ou chamada de procedimento.
            break;
        case SSE:
            handle_conditional_command(); // Comando 'se'.
            break;
        case SENQUANTO:
            handle_while_command(); // Comando 'enquanto'.
            break;
        case SLEIA:
            handle_read_command(); // Comando 'leia'.
            break;
        case SESCREVA:
            handle_write_command(); // Comando 'escreva'.
            break;
        case SINICIO:
            handle_commands(); // Bloco de comandos aninhado.
            break;
        default:
            // Erro: token não inicia um comando válido.
            print_sintax_error(NULL);
            break;
    }
}

/**
 * @brief Distingue entre comando de atribuição e chamada de procedimento após a leitura do identificador.
 * * <atribuição_chprocedimento>::= (<comando atribuicao>| <chamada de procedimento>)
 */
void handle_assignment_chprocedure() {
    Token aux = current_token;
    get_next_token();
    if (current_token.symbol == SATRIBUICAO) {
        handle_assignment_command(aux); // É uma atribuição.
    }
    else {
        handle_procedure_call(aux); // É uma chamada de procedimento.
    }
}

/**
 * @brief Analisa o comando de leitura: <comando leitura>::= leia ( <identificador> )
 * * Realiza a checagem semântica se o identificador é uma variável do tipo INT.
 */
void handle_read_command() {
    get_next_token();
    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            
            if (node != NULL) {
                // Checagem Semântica: Verifica se é uma variável inteira
                if (node->data_type == INT) {

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
                    // Erro Semântico: Não é uma variável inteira.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Erro Semântico: Símbolo não declarado.
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
 * @brief Analisa o comando de escrita: <comando escrita>::= escreva ( <identificador> )
 * * Realiza a checagem semântica se o identificador é uma variável do tipo INT.
 */
void handle_write_command() {
    get_next_token();

    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            
            if (node != NULL) {
                // Checagem Semântica: Verifica se é uma variável inteira e não um marcador de escopo.
                if (node->data_type == INT && node->scope == false) {

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
                    // Erro Semântico: Não é uma variável inteira ou é um símbolo de escopo.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Erro Semântico: Símbolo não declarado.
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
 * @brief Analisa o comando de repetição 'enquanto': <comando enquanto>::= enquanto <expressão> faca <comando>
 * * Realiza a checagem semântica para garantir que a expressão é do tipo BOOL.
 */
void handle_while_command() {
    int l1 = label++; // Rótulo para o início do loop (expressão)
    int l2 = label++; // Rótulo para sair do loop

    // Geração de Código: Ponto de entrada do loop
    generate(l1, "NULL ", -1, -1);

    get_next_token();
    // Checagem Semântica: A expressão deve ser booleana.
    if (handle_expression() != BOOL) { 
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
 * @brief Analisa o comando condicional 'se': <comando condicional>::= se <expressão> entao <comando> [senao <comando>]
 * * Realiza a checagem semântica para garantir que a expressão é do tipo BOOL.
 */
void handle_conditional_command() {
    int l1 = label++; // Rótulo para o bloco 'senao' ou final

    get_next_token();
    // Checagem Semântica: A expressão deve ser booleana.
    if (handle_expression() != BOOL) {
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
 * @brief Analisa a seção de declaração de sub-rotinas (procedimentos e funções).
 * * <etapa de declaração de sub-rotinas> ::= (<declaração de procedimento>;| <declaração de função>;) {...}
 * * @return bool Retorna true se pelo menos uma sub-rotina aninhada foi encontrada, false caso contrário.
 */
bool handle_subroutine_section() {
    bool has_inner_subprogram = false;

    if (current_token.symbol == SPROCEDIMENTO || current_token.symbol == SFUNCAO) {
        // Geração de Código: Inserir um salto para pular a definição da sub-rotina ao executar o bloco principal.
        generate(-1, "JMP  ", label, -1);
        label++;
        has_inner_subprogram = true;
    }

    // Loop para lidar com múltiplas declarações de procedimento ou função.
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

/**
 * @brief Analisa a declaração de um procedimento: <declaração de procedimento> ::= procedimento <identificador>; <bloco>
 */
void handle_procedure_declaration() {
    int l1 = label++; // Rótulo de entrada do procedimento
    SymbolNode * proc;

    get_next_token();

    if (current_token.symbol == SIDENTIFICADOR) {
        // Ação Semântica: Verifica unicidade e insere o procedimento.
        if (can_declare_subroutine(current_token.lexem)) {
            insert_node_table(current_token.lexem, true, UNDEFINED, PROCEDURE, -1);
            proc = table;

            generate(l1, "NULL", -1, -1); // Rótulo de entrada

            get_next_token();
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block(); // Analisa o corpo do procedimento
            } else {
                print_sintax_error(";");
            }
        }
        else {
            // Erro Semântico: Identificador já existe.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s já existe\n", current_line, current_token.lexem);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador do procedimento após 'procedimento'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    // Ação Semântica: Remove o escopo da sub-rotina da pilha.
    pop_scope(proc);
    proc->mem = l1; // Armazena o rótulo de entrada na Tabela de Símbolos
    generate(-1, "RETURN", -1, -1); // Geração de Código: Retorno de procedimento
}

/**
 * @brief Analisa a declaração de uma função: <declaração de função> ::= funcao <identificador>: <tipo>; <bloco>
 */
void handle_function_declaration() {
    int l1 = label++; // Rótulo de entrada da função
    SymbolNode * func;

    get_next_token();
    if (current_token.symbol == SIDENTIFICADOR) {
        // Ação Semântica: Verifica unicidade e insere a função.
        if (can_declare_subroutine(current_token.lexem)) {
            insert_node_table(current_token.lexem, true, UNDEFINED, FUNC, l1);
            func = table;

            generate(l1, "NULL ", -1, -1); // Rótulo de entrada
            
            get_next_token();
            if (current_token.symbol == SDOISPONTOS) {
                get_next_token();
                if (current_token.symbol == SINTEIRO || current_token.symbol == SBOOLEANO) {
                    // Ação Semântica: Atribui o tipo de retorno da função.
                    if (current_token.symbol == SINTEIRO)
                        insert_type(INT);
                    else
                        insert_type(BOOL);
                    
                    get_next_token();
                    if (current_token.symbol == SPONTO_VIRGULA) {
                        get_next_token();
                        handle_block(); // Analisa o corpo da função
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
            // Erro Semântico: Identificador já existe.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: %s já existe\n", current_line, current_token.lexem);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador da função após 'funcao'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    // Ação Semântica: Remove o escopo da sub-rotina da pilha.
    pop_scope(func);
    func->mem = l1; // Armazena o rótulo de entrada na Tabela de Símbolos
    generate(-1, "RETURN", -1, -1); // Geração de Código: Retorno de função
}

/**
 * @brief Função auxiliar para validar que o operador relacional está sendo usado com tipos inteiros.
 * * Consome o token do operador e verifica se a expressão à direita é INT.
 * * @param type1 Ponteiro para o tipo da expressão à esquerda (deve ser INT).
 */
void process_relational_operator(DataType * type1) {
    if (*type1 != INT) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis1\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    get_next_token();
    DataType type2 = handle_simple_expression();
    if (type2 != INT) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis2\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Analisa uma expressão: <expressão>::= <expressão simples> [<operador relacional><expressão simples>]
 * * @return DataType O tipo de dado resultante da expressão (INT ou BOOL).
 */
DataType handle_expression() {
    DataType type1 = handle_simple_expression();
    
    // Verifica a presença de um operador relacional.
    if (current_token.symbol == SMAIOR) {
        process_relational_operator(&type1);
        generate(-1, "CMA  ", -1, -1);
        return BOOL;
    }
    if (current_token.symbol == SMAIORIG) {
        process_relational_operator(&type1);
        generate(-1, "CMAQ ", -1, -1);
        return BOOL;
    }
    if (current_token.symbol == SIG) {
        process_relational_operator(&type1);
        generate(-1, "CEQ  ", -1, -1);
        return BOOL;
    }
    if (current_token.symbol == SMENOR) {
        process_relational_operator(&type1);
        generate(-1, "CME  ", -1, -1);
        return BOOL;
    }
    if (current_token.symbol == SMENORIG) {
        process_relational_operator(&type1);
        generate(-1, "CMEQ ", -1, -1);
        return BOOL;
    }
    if (current_token.symbol == SDIF) {
        process_relational_operator(&type1);
        generate(-1, "CDIF ", -1, -1);
        return BOOL;
    }
    
    return type1; // Retorna o tipo da expressão simples (se não houver relacional).
}

/**
 * @brief Analisa uma expressão simples: <expressão simples> ::= [ + | - ] <termo> {( + | - | ou) <termo> }
 * * Implementa a precedência para operadores aditivos (+, -) e OR (ou).
 * Realiza as checagens de compatibilidade de tipos.
 * * @return DataType O tipo de dado resultante (INT para +, -; BOOL para 'ou').
 */
DataType handle_simple_expression() {
    DataType ref = UNDEFINED;

    // Checa a presença de sinal unário opcional (+ ou -).
    if (current_token.symbol == SMAIS || current_token.symbol == SMENOS) {
        get_next_token();
        ref = INT; // Sinais unários implicam tipo inteiro.
    }

    DataType type = handle_term(); // Analisa o primeiro termo

    // Checagem de tipo para o primeiro termo.
    if (type == INT) {
        ref = INT;
    } else {
        if (ref == INT) {
            // Erro Semântico: Sinal unário com termo não-inteiro.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis3\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        } else {
            ref = BOOL; // Se não tem sinal unário, o tipo é determinado pelo termo (BOOL).
        }
    }

    // Armazena os operadores para gerar o código na ordem correta (in-order).
    Simbolo * operations = (Simbolo*)malloc(100 * sizeof(Simbolo));
    int i_op = 0;

    // Loop para operações aditivas/OR.
    while (current_token.symbol == SMAIS || current_token.symbol == SMENOS || current_token.symbol == SOU) {
        operations[i_op] = current_token.symbol;
        i_op++;

        // Checagem Semântica: Operador 'ou' com tipo inteiro é um erro.
        if (current_token.symbol == SOU && ref == INT) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis4\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        // Checagem Semântica: Operadores aditivos (+, -) com tipo booleano é um erro.
        else if ((current_token.symbol == SMAIS || current_token.symbol == SMENOS) && ref == BOOL) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis5\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        get_next_token();
        // Checagem Semântica: O termo subsequente deve ser do mesmo tipo.
        if (handle_term() != ref) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis6\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // Geração de Código: Executa as operações aditivas/OR na ordem.
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
    return ref; // Retorna o tipo da expressão simples.
}

/**
 * @brief Analisa um termo: <termo>::= <fator> {(* | div | e) <fator>}
 * * Implementa a precedência para operadores multiplicativos (*, div) e AND (e).
 * Realiza as checagens de compatibilidade de tipos.
 * * @return DataType O tipo de dado resultante (INT para *, div; BOOL para 'e').
 */
DataType handle_term() {
    DataType type1 = handle_factor();
    Simbolo s = current_token.symbol;

    // Armazena os operadores para gerar o código na ordem correta (in-order).
    Simbolo * operations = (Simbolo*)malloc(100 * sizeof(Simbolo));
    int i_op = 0;

    DataType type_to_return = -1;

    // Loop para operações multiplicativas/AND.
    while(current_token.symbol == SMULT || current_token.symbol == SDIV || current_token.symbol == SE) {
        operations[i_op] = current_token.symbol;
        i_op++;

        get_next_token();
        DataType type2 = handle_factor();
        
        // Checagem Semântica: Multiplicação/Divisão deve ser com tipos INT.
        if (s == SMULT || s == SDIV) {
            if (type1 != INT || type2 != INT) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação numérica com booleano\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else {
                type_to_return = INT; // Resultado é INT.
            }
        }
        // Checagem Semântica: Operador 'e' (AND) deve ser com tipos BOOL.
        else {
            if (type1 != BOOL || type2 != BOOL) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação lógica com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else {
                type_to_return = BOOL; // Resultado é BOOL.
            }
        }
        s = current_token.symbol;
    }

    // Geração de Código: Executa as operações multiplicativas/AND na ordem.
    for (int i = 0; i < i_op; i++) {
        switch (operations[i]) {
            case SMULT:
                generate(-1, "MULT ", -1, -1);
                break;
            case SDIV:
                generate(-1, "DIV  ", -1, -1);
                break;
            case SE:
                generate(-1, "AND  ", -1, -1);
                break;
            default:
                break;
        }
    }

    free(operations);

    if (type_to_return == INT || type_to_return == BOOL) {
        return type_to_return;
    } else {
        return type1; // Retorna o tipo do fator se não houve operação.
    }
}

/**
 * @brief Analisa um fator: <fator> ::= (<variável> | <número> | <chamada de função> | (<expressão>) | verdadeiro | falso | nao <fator>)
 * * Trata o menor nível de expressões.
 * * @return DataType O tipo de dado do fator.
 */
DataType handle_factor() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            {
                SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
                
                if (node != NULL) {
                    if (node->structure_type == FUNC) { // É uma função
                        handle_function_call(node);
                    } 
                    else if (node->structure_type == VAR) { // É uma variável
                        handle_variable(node);
                    }
                    else {
                        // Erro Semântico: Identificador existe, mas não é VAR nem FUNC (ex: nome de procedimento ou programa)
                        fprintf(stderr, "ERRO SEMANTICO na linha %d: %s não é uma variável ou função\n", current_line, current_token.lexem);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    }
                    return node->data_type; // Retorna o tipo do símbolo.
                }
                else {
                    // Erro Semântico: Símbolo não existe.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: %s não existe\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case SNUMERO:
            // Geração de Código: Carrega a constante na pilha.
            generate(-1, "LDC  ", atoi(current_token.lexem), -1);
            get_next_token();
            return INT;
        case SVERDADEIRO:
            // Geração de Código: Carrega a constante booleana 'verdadeiro' (1).
            generate(-1, "LDC  ", 1, -1);
            get_next_token();
            return BOOL;
        case SFALSO:
            // Geração de Código: Carrega a constante booleana 'falso' (0).
            generate(-1, "LDC  ", 0, -1);
            get_next_token();
            return BOOL;
        case SNAO: // Operador NOT
            get_next_token();
            // Checagem Semântica: 'nao' deve ser usado com tipo BOOL.
            if (handle_factor() == BOOL) {
                generate(-1, "NEG  ", -1, -1); // Geração de Código: Negação lógica
                return BOOL;
            }
            else {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operador \"n\" não usado com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        case SABRE_PARENTESES:
            get_next_token();
            DataType tipo = handle_expression(); // Analisa expressão dentro de parênteses

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
}

/**
 * @brief Analisa o comando de atribuição: <comando atribuicao>::= <identificador> := <expressão>
 * * O identificador já foi reconhecido na chamada a `handle_assignment_chprocedure`.
 * * @param aux O Token do identificador a ser atribuído.
 */
void handle_assignment_command(Token aux) {
    get_next_token();

    SymbolNode * obj_to_assign_value = get_symbol_from_lexem(aux.lexem);

    // Checagem Semântica: Verifica a compatibilidade de tipos na atribuição.
    DataType t1 = handle_expression();
    DataType t2 = obj_to_assign_value->data_type;

    if (t1 != t2) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: tipos incompatíveis na atribuição\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    } 

    // Geração de Código: Armazena o valor da expressão no endereço de memória do símbolo.
    if (obj_to_assign_value->structure_type == FUNC) {
        // Atribuição ao resultado da função (endereço 0)
        generate(-1, "STR  ", 0, -1);
    } else {
        // Atribuição a uma variável local/global
        generate(-1, "STR  ", obj_to_assign_value->mem, -1);
    }
}

/**
 * @brief Analisa a chamada de procedimento: <chamada de procedimento>::= <identificador>
 * * @param aux O Token do identificador do procedimento.
 */
void handle_procedure_call(Token aux) {
    SymbolNode * node = get_symbol_from_lexem(aux.lexem);
    if (node != NULL) {
        generate(-1, "CALL ", node->mem, -1);
    }
    else {
        // Erro Semântico: Identificador não existe.
        fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s não existe\n", current_line, aux.lexem);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Analisa a utilização de uma variável em uma expressão: <variável> ::= <identificador>
 * * @param node O nó do símbolo da variável.
 */
void handle_variable(SymbolNode * node) {
    // Geração de Código: Carrega o valor da variável na pilha.
    generate(-1, "LDV  ", node->mem, -1);

    get_next_token();
}

/**
 * @brief Analisa a chamada de função em uma expressão: <chamada de função> ::= <identificador>
 * * @param node O nó do símbolo da função.
 */
void handle_function_call(SymbolNode * node) {
    // Geração de Código: Chama a função.
    generate(-1, "CALL ", node->mem, -1);
    // Geração de Código: Carrega o resultado da função (endereço 0) na pilha.
    generate(-1, "LDV  ", 0, -1);

    get_next_token();
}