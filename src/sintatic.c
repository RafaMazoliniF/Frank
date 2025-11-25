/**
 * @file sintatic.c
 * @brief Implementação do Analisador Sintático (Parser) com verificação semântica e geração de código intermediário (MEPA).
 */
#include "sintatic.h"
#include "codegen.h"
#include "lexical.h"
#include "semantic.h"

// Protótipo para uso interno
void print_sintax_error(const char* expected);

/**
 * @brief Reporta erro sintático e aborta a execução.
 * @param expected Token esperado (opcional) para mensagem detalhada.
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
 * @brief Regra: <programa> ::= programa <identificador> ; <bloco> .
 * Gerencia o ciclo de vida principal do programa e gera instruções de inicialização.
 */
void handle_program() {
    label = 1;
    // CodeGen: Inicializa a máquina virtual MEPA
    generate(-1, "START", -1, -1);
    // CodeGen: Aloca espaço para a variável de retorno da função principal (endereço 0)
    generate(-1, "ALLOC", 0, 1); 

    get_next_token();

    if (current_token.symbol == SPROGRAMA) {
        get_next_token();

        if (current_token.symbol == SIDENTIFICADOR) {
            // Semântica: Registra escopo global e o nome do programa na tabela de símbolos
            insert_node_table(current_token.lexem, true, UNDEFINED, PROGRAM_NAME, -1);
            get_next_token();

            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block(); // Processa o bloco principal (declarações e comandos) 

                if (current_token.symbol == SPONTO) {
                    get_next_token();

                    // Validação de fim de arquivo (ENDFILE)
                    if (current_token.lexem != NULL || current_token.symbol != ENDFILE) {
                        fprintf(stderr, "ERRO SINTÁTICO na linha %d: código após o fim do programa\n", current_line);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    } else {
                        // CodeGen: Desalocação da variável de retorno e finalização
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
 * @brief Regra: <bloco> ::= [<parte declaração de variáveis>] [<parte declaração de sub-rotinas>] <comando composto>
 * Gerencia escopo, alocação de variáveis locais e fluxo de sub-rotinas.
 */
void handle_block() {
    int count = handle_variable_declaration_section();
    int addr_to_dealloc = addr;
    
    // CodeGen: Alocação de 'count' variáveis locais a partir do endereço 'addr'
    if (count > 0) {
        generate(-1, "ALLOC", addr, count);
        addr += count; // Atualiza o próximo endereço livre
    }

    int aux_label = label;
    // Processa procedimentos e funções aninhados
    bool has_inner_subprogram = handle_subroutine_section();

    // CodeGen: Se houver sub-rotinas, o JMP gerado em handle_subroutine_section 
    // desvia para este NULL, pulando as declarações para executar o bloco principal.
    if (has_inner_subprogram) {
        generate(aux_label, "NULL ", -1, -1);
    }

    handle_commands(); // Processa a seção de comandos
    
    // CodeGen: Desalocação das variáveis locais ao sair do escopo
    if (count > 0) {
        generate(-1, "DALLOC", addr_to_dealloc, count);
        addr -= count; // Retorna o endereço livre ao estado anterior
    }
}

/**
 * @brief Processa declarações de variáveis (var ...).
 * @return Número total de variáveis declaradas neste bloco.
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
 * @brief Regra: <variáveis> ::= <identificador> {, <identificador>} : <tipo>
 * @param count Ponteiro para o contador de variáveis.
 */
void handle_variables(int * count) {
    do {
        if (current_token.symbol == SIDENTIFICADOR) {
            // Semântica: Validação de duplicidade no escopo atual
            if (can_declare_variable(current_token.lexem)) {
                // Semântica: Insere na tabela como VAR, com endereço relativo atual
                insert_node_table(current_token.lexem, false, UNDEFINED, VAR, addr + (*count));
                *count += 1;

                get_next_token();

                if (current_token.symbol == SVIRGULA) {
                    get_next_token();
                    // Validação sintática: garantir que após ',' haja 'identificador' ou ':'
                    if (current_token.symbol != SIDENTIFICADOR && current_token.symbol != SDOISPONTOS) {
                        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador após ','\n", current_line);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    }
                } else if (current_token.symbol == SDOISPONTOS) {
                    break; // Fim da lista de variáveis, esperado tipo
                } else {
                    print_sintax_error(",' ou ':");
                }
            }
            else {
                // Erro Semântico: Variável já declarada
                fprintf(stderr, "ERRO SEMANTICO na linha %d: variável %s já existe\n", current_line, current_token.lexem);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        } else {
            print_sintax_error("identificador");
        }
    } while (current_token.symbol != SDOISPONTOS);

    get_next_token(); // Consome ':'
    handle_type(); // Aplica o tipo às variáveis recém-inseridas
}

/**
 * @brief Regra: <tipo> ::= inteiro | booleano
 * Aplica o tipo detectado aos símbolos pendentes na tabela.
 */
void handle_type() {
    if (current_token.symbol != SINTEIRO && current_token.symbol != SBOOLEANO) {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado tipo 'inteiro' ou 'booleano'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    else {
        // Semântica: Aplica o tipo (TYPE_INT ou TYPE_BOOL) aos nós pendentes na tabela
        if (current_token.symbol == SINTEIRO)
            insert_type(TYPE_INT);
        else 
            insert_type(TYPE_BOOL);
    }
    get_next_token(); // Consome o tipo
}

/**
 * @brief Regra: <comando composto> ::= inicio <comando> {; <comando>} fim
 */
void handle_commands() {
    if (current_token.symbol == SINICIO) {
        get_next_token();
        handle_command(); // Primeiro comando

        while (current_token.symbol != SFIM) {
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                // Permite ';' opcional antes do 'fim'
                if (current_token.symbol != SFIM) {
                    handle_command();
                }
            } else {
                print_sintax_error(";");
            }
        }
        get_next_token(); // Consome 'fim'

    } else {
        print_sintax_error("inicio");
    }
}

/**
 * @brief Dispatcher para os diversos tipos de comandos suportados.
 */
void handle_command() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            handle_assignment_chprocedure(); // Atribuição ou chamada de procedimento
            break;
        case SSE:
            handle_conditional_command(); // Comando condicional 'se'
            break;
        case SENQUANTO:
            handle_while_command(); // Comando de repetição 'enquanto'
            break;
        case SLEIA:
            handle_read_command(); // Comando de leitura 'leia'
            break;
        case SESCREVA:
            handle_write_command(); // Comando de escrita 'escreva'
            break;
        case SINICIO:
            handle_commands(); // Comando composto (bloco interno)
            break;
        default:
            print_sintax_error(NULL);
            break;
    }
}

// Decide entre atribuição ou chamada de procedimento (Lookahead: :=)
void handle_assignment_chprocedure() {
    Token aux = current_token;
    get_next_token();
    // Se o próximo for ':=', é atribuição
    if (current_token.symbol == SATRIBUICAO) {
        handle_assignment_command(aux); 
    }
    // Caso contrário, é chamada de procedimento
    else {
        handle_procedure_call(aux); 
    }
}

/**
 * @brief Regra: <comando leitura> ::= leia ( <identificador> )
 * Lê entrada padrão e armazena na memória.
 */
void handle_read_command() {
    get_next_token(); // Consome 'leia'
    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            
            if (node != NULL) {
                // Semântica: Apenas variáveis do tipo inteiro podem ser lidas
                if (node->data_type == TYPE_INT) {

                    // CodeGen: Leitura (RD) e armazenamento (STR) no endereço da variável
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
                    // Erro Semântico: Leitura de tipo não inteiro
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Erro Semântico: Símbolo não existe
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
 * @brief Regra: <comando escrita> ::= escreva ( <identificador> )
 * Carrega variável e imprime na saída padrão.
 */
void handle_write_command() {
    get_next_token(); // Consome 'escreva'

    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            
            if (node != NULL) {
                // Semântica: Apenas variáveis inteiras (e não de escopo) podem ser escritas
                if (node->data_type == TYPE_INT && node->scope == false) {

                    // CodeGen: Carregar valor (LDV) do endereço e Imprimir (PRN)
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
                    // Erro Semântico: Escrita de tipo não inteiro
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Erro Semântico: Símbolo não existe
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
 * @brief Regra: <comando repetição> ::= enquanto <expressão> faca <comando>
 * Implementa loop com verificação no início (while).
 */
void handle_while_command() {
    int l1 = label++; // Rótulo de teste (início do loop)
    int l2 = label++; // Rótulo de saída (após o loop)

    generate(l1, "NULL ", -1, -1); // CodeGen: Marca o ponto de teste

    get_next_token(); // Consome 'enquanto'
    // Semântica: Expressão deve resultar em booleano
    if (handle_expression() != TYPE_BOOL) { 
        fprintf(stderr, "ERRO SEMANTICO na linha %d: expressão deve ser BOOL\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    } 

    // CodeGen: Compara resultado (topo da pilha) com 'falso' (0) e desvia se for falso
    // LDC 1: Coloca 'verdadeiro' na pilha
    // CEQ: Compara (topo-1) com (topo), se forem iguais, resulta 1 (verdadeiro).
    // O JMPF desvia se o resultado da CEQ for falso (0), ou seja, se a expressão original for verdadeira.
    // É uma implementação invertida de JMPF: Se a expressão for F, desvia (LDC 1 CEQ JMPF L2)
    // Se a expressão for V, (resultado 1), LDC 1 CEQ (0), JMPF (não desvia)
    generate(-1, "LDC  ", 0, -1); // Coloca FALSO na pilha
    generate(-1, "CEQ", -1, -1); // Compara resultado da expressão com FALSO. Se igual a FALSO (0), CEQ = 1 (V). Se igual a V (1), CEQ = 0 (F).
    generate(-1, "JMPF ", l2, -1); // Se CEQ = F (0), a expressão original era V, então executa. Se CEQ = V (1), a expressão original era F, então PULA para L2.

    if (current_token.symbol == SFACA) {
        get_next_token();
        handle_command(); // Corpo do loop
    } else {
        print_sintax_error("faca");
    }

    generate(-1, "JMP  ", l1, -1); // CodeGen: Retorno ao teste
    generate(l2, "NULL ", -1, -1); // CodeGen: Ponto de saída do loop
}

/**
 * @brief Regra: <comando condicional> ::= se <expressão> entao <comando> [senao <comando>]
 */
void handle_conditional_command() {
    int l1 = label++; // Rótulo para 'senao' ou fim do 'se'

    get_next_token(); // Consome 'se'
    // Semântica: Expressão deve resultar em booleano
    if (handle_expression() != TYPE_BOOL) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: expressão deve ser BOOL\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    // CodeGen: Salta para L1 se expressão for falsa (executa o 'senao' ou pula para o fim)
    generate(-1, "JMPF ", l1, -1);

    if (current_token.symbol == SENTAO) {
        get_next_token();
        handle_command(); // Bloco 'entao' 

        if (current_token.symbol == SSENAO) {
            int l2 = label++; // Rótulo para fim absoluto (após 'senao')

            // CodeGen: Pula o bloco 'senao' após executar 'entao'
            generate(-1, "JMP  ", l2, -1); 
            generate(l1, "NULL ", -1, -1); // CodeGen: Início do bloco 'senao' (alvo do JMPF inicial)

            get_next_token(); // Consome 'senao'
            handle_command(); // Bloco 'senao' 

            generate(l2, "NULL ", -1, -1); // CodeGen: Fim do comando condicional
        } else {
            generate(l1, "NULL ", -1, -1); // CodeGen: Alvo do JMPF se não houver 'senao'
        }
    } else {
        print_sintax_error("entao");
    }
}

/**
 * @brief Processa declarações de Procedimentos e Funções.
 * @return true se houver sub-rotinas (exige salto no código principal), false caso contrário.
 */
bool handle_subroutine_section() {
    bool has_inner_subprogram = false;

    if (current_token.symbol == SPROCEDIMENTO || current_token.symbol == SFUNCAO) {
        // CodeGen: Se há sub-rotinas, gera um salto JMP para pular suas definições
        generate(-1, "JMP  ", label, -1);
        label++; // 'label' agora será o alvo do JMP (o 'NULL' no final de handle_block)
        has_inner_subprogram = true;
    }

    // Processa todas as declarações de sub-rotinas
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
 * @brief Regra: procedimento <identificador> ; <bloco>
 */
void handle_procedure_declaration() {
    int l1 = label++; // Rótulo de entrada do procedimento
    SymbolNode * proc;

    get_next_token(); // Consome 'procedimento'

    if (current_token.symbol == SIDENTIFICADOR) {
        if (can_declare_subroutine(current_token.lexem)) {
            // Semântica: Insere na tabela como PROCEDURE. 'mem' será o endereço de entrada
            insert_node_table(current_token.lexem, true, UNDEFINED, PROCEDURE, -1);
            proc = table; // Guarda ponteiro para o nó

            generate(l1, "NULL", -1, -1); // CodeGen: Marca o ponto de entrada (rótulo l1)

            get_next_token();
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block(); // Processa o corpo do procedimento
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
    pop_scope(proc); // Semântica: Limpa variáveis locais e escopo da tabela
    proc->mem = l1;  // Semântica: Atualiza o endereço de entrada da rotina
    generate(-1, "RETURN", -1, -1); // CodeGen: Retorna da sub-rotina 
}

/**
 * @brief Regra: funcao <identificador> : <tipo> ; <bloco>
 */
void handle_function_declaration() {
    int l1 = label++; // Rótulo de entrada da função
    SymbolNode * func;

    get_next_token(); // Consome 'funcao'
    if (current_token.symbol == SIDENTIFICADOR) {
        if (can_declare_subroutine(current_token.lexem)) {
            // Semântica: Insere na tabela como FUNC. 'mem' será o endereço de entrada
            insert_node_table(current_token.lexem, true, UNDEFINED, FUNC, l1);
            func = table;

            generate(l1, "NULL ", -1, -1); // CodeGen: Marca o ponto de entrada (rótulo l1)
            
            get_next_token();
            if (current_token.symbol == SDOISPONTOS) {
                get_next_token();
                if (current_token.symbol == SINTEIRO || current_token.symbol == SBOOLEANO) {
                    // Semântica: Aplica o tipo de retorno da função
                    if (current_token.symbol == SINTEIRO)
                        insert_type(TYPE_INT);
                    else 
                        insert_type(TYPE_BOOL);
                    
                    get_next_token();
                    if (current_token.symbol == SPONTO_VIRGULA) {
                        get_next_token();
                        handle_block(); // Processa o corpo da função
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
    pop_scope(func); // Semântica: Limpa variáveis locais e escopo da tabela
    func->mem = l1;  // Semântica: Atualiza o endereço de entrada da rotina
    generate(-1, "RETURN", -1, -1); // CodeGen: Retorna da sub-rotina
}

// Auxiliar para verificação de tipos em operações relacionais
void process_relational_operator(DataType * type1) {
    // Semântica: O primeiro operando deve ser inteiro
    if (*type1 != TYPE_INT) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis1\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    get_next_token(); // Consome o operador relacional
    DataType type2 = handle_simple_expression();
    // Semântica: O segundo operando também deve ser inteiro
    if (type2 != TYPE_INT) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis2\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Regra: <expressão> ::= <expressão simples> [<op relacional> <expressão simples>]
 * Analisa expressões (prioridade menor: relacionais).
 * @return Tipo resultante da expressão (TYPE_BOOL ou TYPE_INT).
 */
DataType handle_expression() {
    DataType type1 = handle_simple_expression();
    
    // Processamento de operadores relacionais (>, >=, =, <, <=, !=)
    if (current_token.symbol == SMAIOR) {
        process_relational_operator(&type1);
        generate(-1, "CMA  ", -1, -1); // CodeGen: Compara Maior (>)
        return TYPE_BOOL;
    }
    if (current_token.symbol == SMAIORIG) {
        process_relational_operator(&type1);
        generate(-1, "CMAQ ", -1, -1); // CodeGen: Compara Maior ou Igual (>=)
        return TYPE_BOOL;
    }
    if (current_token.symbol == SIG) {
        process_relational_operator(&type1);
        generate(-1, "CEQ  ", -1, -1); // CodeGen: Compara Igual (=)
        return TYPE_BOOL;
    }
    if (current_token.symbol == SMENOR) {
        process_relational_operator(&type1);
        generate(-1, "CME  ", -1, -1); // CodeGen: Compara Menor (<)
        return TYPE_BOOL;
    }
    if (current_token.symbol == SMENORIG) {
        process_relational_operator(&type1);
        generate(-1, "CMEQ ", -1, -1); // CodeGen: Compara Menor ou Igual (<=)
        return TYPE_BOOL;
    }
    if (current_token.symbol == SDIF) {
        process_relational_operator(&type1);
        generate(-1, "CDIF ", -1, -1); // CodeGen: Compara Diferente (!=)
        return TYPE_BOOL;
    }
    
    // Se não houver operador relacional, o tipo é o da expressão simples
    return type1; 
}

/**
 * @brief Regra: <expressão simples> ::= [+|-] <termo> {<op aditivo> <termo>}
 * Analisa expressões simples (aditivos: +, -, ou).
 */
DataType handle_simple_expression() {
    DataType ref = UNDEFINED;
    bool neg = false;

    // Processamento de sinais unários (+ ou -)
    if (current_token.symbol == SMAIS || current_token.symbol == SMENOS) {
        if (current_token.symbol == SMENOS) {
            neg = true;
        }
        get_next_token();
        ref = TYPE_INT; // Unário só se aplica a tipo inteiro
    }

    DataType type = handle_term(); // Primeiro termo

    if (type == TYPE_INT) {
        if (neg) {
            generate(-1, "NEG  ", -1, -1); // CodeGen: Negação unária
        }
        ref = TYPE_INT;
    } else {
        // Se o termo não é inteiro, e havia um unário (+/-), é erro
        if (ref == TYPE_INT) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis3\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        } else {
            ref = TYPE_BOOL; // Expressão começa com um fator booleano
        }
    }

    // Armazenamento de operadores para código pós-fixo
    Simbolo * operations = (Simbolo*)malloc(100 * sizeof(Simbolo));
    int i_op = 0;

    // Loop para processar termos aditivos subsequentes
    while (current_token.symbol == SMAIS || current_token.symbol == SMENOS || current_token.symbol == SOU) {
        operations[i_op] = current_token.symbol;
        i_op++;

        // Semântica: Validação de compatibilidade de tipos
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
        // O termo subsequente deve ter o mesmo tipo de 'ref'
        if (handle_term() != ref) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis6\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // CodeGen: Aplica operações aditivas/lógicas OU pendentes (pós-fixa)
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
 * @brief Regra: <termo> ::= <fator> {<op mult> <fator>}
 * Analisa termos (multiplicativos: *, /, e).
 */
DataType handle_term() {

    DataType type1 = handle_factor(); // Primeiro fator

    Simbolo s = current_token.symbol;

    // Armazenamento de operadores para código pós-fixo
    Simbolo * operations = (Simbolo*)malloc(100 * sizeof(Simbolo));
    int i_op = 0;

    DataType type_to_return = UNDEFINED;

    // Loop para processar fatores multiplicativos subsequentes
    while(current_token.symbol == SMULT || current_token.symbol == SDIV || current_token.symbol == SE) {
        operations[i_op] = current_token.symbol;
        i_op++;

        get_next_token();
        DataType type2 = handle_factor();        

        // Semântica: Validação Aritmética vs Lógica (Multiplicação/Divisão vs E)
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
        else { // Operador 'e'
            if (type1 != TYPE_BOOL || type2 != TYPE_BOOL) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação lógica com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else {
                type_to_return = TYPE_BOOL; 
            }
        }
        s = current_token.symbol; // Atualiza o símbolo para a próxima iteração
    }

    // CodeGen: Aplica operações multiplicativas/lógicas E pendentes
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

    // Retorna o tipo se alguma operação ocorreu, senão retorna o tipo do primeiro fator
    if (type_to_return == TYPE_INT || type_to_return == TYPE_BOOL) {
        return type_to_return;
    } else {
        return type1; 
    }

}

/**
 * @brief Regra: <fator> ::= <identificador> | <número> | verdadeiro | falso | nao <fator> | ( <expressão> )
 * Nível mais alto de precedência.
 * @return Tipo resultante do fator.
 */
DataType handle_factor() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            {
                SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
                
                if (node != NULL) {
                    // Se é função, chama a rotina
                    if (node->structure_type == FUNC) { 
                        handle_function_call(node);
                    } 
                    // Se é variável, carrega o valor
                    else if (node->structure_type == VAR) { 
                        handle_variable(node);
                    }
                    else {
                        // Erro Semântico: Identificador não é variável nem função
                        fprintf(stderr, "ERRO SEMANTICO na linha %d: %s não é uma variável ou função\n", current_line, current_token.lexem);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    }
                    return node->data_type; // Retorna o tipo do símbolo
                }
                else {
                    // Erro Semântico: Símbolo não existe
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: %s não existe\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case SNUMERO:
            // CodeGen: Carrega constante numérica (Load Constant)
            generate(-1, "LDC  ", atoi(current_token.lexem), -1);
            get_next_token();
            return TYPE_INT;
        case SVERDADEIRO:
            // CodeGen: Carrega constante booleana Verdadeiro (1)
            generate(-1, "LDC  ", 1, -1);
            get_next_token();
            return TYPE_BOOL;
        case SFALSO:
            // CodeGen: Carrega constante booleana Falso (0)
            generate(-1, "LDC  ", 0, -1);
            get_next_token();
            return TYPE_BOOL;
        case SNAO: 
            get_next_token(); // Consome 'nao'
            if (handle_factor() == TYPE_BOOL) {
                generate(-1, "NEG  ", -1, -1); // CodeGen: Negação lógica (0 vira 1, 1 vira 0)
                return TYPE_BOOL;
            }
            else {
                // Erro Semântico: Negação aplicada a não-booleano
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operador \"n\" não usado com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        case SABRE_PARENTESES:
            get_next_token();
            DataType tipo = handle_expression(); // Expressão entre parênteses

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
    return UNDEFINED; // Não deve ser alcançado em código válido
}

/**
 * @brief Processa atribuição de valor a variável ou retorno de função.
 * Regra: <identificador> := <expressão>
 */
void handle_assignment_command(Token aux) {
    get_next_token(); // Consome ':='

    SymbolNode * obj_to_assign_value = get_symbol_from_lexem(aux.lexem);

    // Semântica: Verifica se o identificador existe
    if (obj_to_assign_value == NULL) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s não existe\n", current_line, aux.lexem);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    DataType t1 = handle_expression(); // Tipo da expressão
    DataType t2 = obj_to_assign_value->data_type; // Tipo da variável/função

    // Semântica: Checagem de compatibilidade de tipos
    if (t1 != t2) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: tipos incompatíveis na atribuição\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    } 

    // CodeGen: Armazena o valor (topo da pilha) no endereço
    if (obj_to_assign_value->structure_type == FUNC) {
        // Se for uma função, armazena no registrador de retorno (endereço 0)
        generate(-1, "STR  ", 0, -1); 
    } else {
        // Se for variável, armazena no endereço da variável
        generate(-1, "STR  ", obj_to_assign_value->mem, -1);
    }
}

/**
 * @brief Processa chamada de procedimento.
 * Regra: <identificador>
 */
void handle_procedure_call(Token aux) {
    SymbolNode * node = get_symbol_from_lexem(aux.lexem);
    if (node != NULL) {
        // CodeGen: Chamada de procedimento (endereço de entrada)
        generate(-1, "CALL ", node->mem, -1);
    }
    else {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s não existe\n", current_line, aux.lexem);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Carrega o valor de uma variável.
 */
void handle_variable(SymbolNode * node) {
    // CodeGen: Carrega valor (Load Value) da variável para o topo da pilha
    generate(-1, "LDV  ", node->mem, -1);
    get_next_token();
}

/**
 * @brief Processa chamada de função.
 */
void handle_function_call(SymbolNode * node) {
    // CodeGen: Chamada de função (endereço de entrada)
    generate(-1, "CALL ", node->mem, -1);
    // CodeGen: Carrega o valor de retorno (armazenado no endereço 0)
    generate(-1, "LDV  ", 0, -1); 
    get_next_token();
}