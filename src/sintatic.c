#include "sintatic.h"
#include "codegen.h"
#include "lexical.h"
#include "semantic.h"

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

// <programa>::= programa <identificador> ; <bloco> .
void handle_program() {
    label = 1;
    generate(-1, "START", -1, -1);
    generate(-1, "ALLOC", 0, 1);

    get_next_token();

    if (current_token.symbol == SPROGRAMA) {
        get_next_token();

        if (current_token.symbol == SIDENTIFICADOR) {
            // Semantic action: Insert program name into symbol table.
            insert_node_table(current_token.lexem, true, UNDEFINED, PROGRAM_NAME, -1);
            get_next_token();

            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block(true);

                if (current_token.symbol == SPONTO) {
                    get_next_token();

                    if (current_token.lexem != NULL || current_token.symbol != ENDFILE) {
                        fprintf(stderr, "ERRO SINTÁTICO na linha %d: código após o fim do programa\n", current_line);
                        fflush(stderr);
                        exit(EXIT_FAILURE);
                    } else {
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

// <bloco>::= [<etapa de declaração de variáveis>]
//           [<etapa de declaração de sub-rotinas>]
//           <comandos>
void handle_block(bool is_main) {
    int count = handle_variable_declaration_section();
    int addr_to_dealloc = addr;
    
    if (count > 0) {
        generate(-1, "ALLOC", addr, count);
        addr += count;
    }

    int aux_label = label;
    bool has_inner_subprogram = handle_subroutine_section();

    if (has_inner_subprogram) {
        generate(aux_label, "NULL ", -1, -1);
    }

    handle_commands();
    if (count > 0) {
        generate(-1, "DALLOC", addr_to_dealloc, count);
        addr -= count;
    }
}

// <etapa de declaração de variáveis>::= var <declaração de variáveis> ;
//                                     {<declaração de variáveis>;}
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

// <declaração de variáveis>::= <identificador> {, <identificador>} : <tipo>
void handle_variables(int * count) {
    do {
        if (current_token.symbol == SIDENTIFICADOR) {
            // Semantic action: Check if variable can be declared and insert into table.
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
                // Semantic error: Variable already exists.
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
    handle_type();
}


// <tipo> ::= (inteiro | booleano)
void handle_type() {
    if (current_token.symbol != SINTEIRO && current_token.symbol != SBOOLEANO) {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado tipo 'inteiro' ou 'booleano'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    else {
        // Semantic action: Insert type (INT or BOOL).
        if (current_token.symbol == SINTEIRO)
            insert_type(INT);
        else 
            insert_type(BOOL);
    }
    get_next_token();
}

// <comandos>::= inicio
//               <comando>{;<comando>}[;]
//              fim
void handle_commands() {
    if (current_token.symbol == SINICIO) {
        get_next_token();
        handle_command();

        while (current_token.symbol != SFIM) {
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                // Handle optional trailing semicolon before 'fim'
                if (current_token.symbol != SFIM) {
                    handle_command();
                }
            } else {
                // Error: token is not 'fim' and not ';', so it's a syntax error.
                print_sintax_error(";");
            }
        }
        // Consume 'fim' token.
        get_next_token();

    } else {
        print_sintax_error("inicio");
    }
}

// <comando>
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
        // Nested block command: <comandos>
        case SINICIO:
            handle_commands();
            break;
        default:
            // Error: token does not start a valid command.
            print_sintax_error(NULL);
            break;
    }
}

// <atribuição_chprocedimento>::= (<comando atribuicao>| <chamada de procedimento>)
void handle_assignment_chprocedure() {
    Token aux = current_token;
    get_next_token();
    if (current_token.symbol == SATRIBUICAO) {
        handle_assignment_command(aux);
    }
    else {
        // It's a procedure call.
        handle_procedure_call(aux);
    }
}

// <comando leitura>::= leia ( <identificador> )
void handle_read_command() {
    get_next_token();
    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            // Semantic action: Check if 'identificador' is an integer variable.
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            if (node != NULL) {
                if (node->data_type == INT) {

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
                    // Semantic error: Not an integer variable.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Semantic error: Symbol does not exist.
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

// <comando escrita>::= escreva ( <identificador> )
void handle_write_command() {
    get_next_token();

    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (current_token.symbol == SIDENTIFICADOR) {
            // Semantic action: Check if 'identificador' is an integer variable.
            SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
            if (node != NULL) {
                if (node->data_type == INT && node->scope == false) {

                    generate(-1, "LDV", node->mem, -1);
                    generate(-1, "PRN", -1, -1);

                    get_next_token();
                    if (current_token.symbol == SFECHA_PARENTESES) {
                        get_next_token();
                    } else {
                        print_sintax_error(")");
                    }
                }
                else {
                    // Semantic error: Not an integer variable.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: símbolo %s não é uma variável inteira\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Semantic error: Symbol does not exist.
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

// <comando enquanto>::= enquanto <expressão> faca <comando>
void handle_while_command() {
    int l1 = label++; 
    int l2 = label++;

    generate(l1, "NULL ", -1, -1);

    get_next_token();
    if (handle_expression() != BOOL) { 
        fprintf(stderr, "ERRO SEMANTICO na linha %d: expressão deve ser BOOL\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    } 

    generate(-1, "LDC  ", 1, -1);
    generate(-1, "CEQ", -1, -1);
    generate(-1, "JMPF ", l2, -1);

    if (current_token.symbol == SFACA) {
        get_next_token();
        handle_command();
    } else {
        print_sintax_error("faca");
    }

    generate(-1, "JMP  ", l1, -1);
    generate(l2, "NULL ", -1, -1);
}

// <comando condicional>::= se <expressão> entao <comando> [senao <comando>]
void handle_conditional_command() {
    int l1 = label++;

    get_next_token();
    if (handle_expression() != BOOL) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: expressão deve ser BOOL\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    generate(-1, "LDC  ", 1, -1);
    generate(-1, "CEQ  ", -1, -1);
    generate(-1, "JMPF ", l1, -1);

    if (current_token.symbol == SENTAO) {
        get_next_token();
        handle_command();

        if (current_token.symbol == SSENAO) {
            int l2 = label++;

            generate(-1, "JMP  ", l2, -1);
            generate(l1, "NULL ", -1, -1);

            get_next_token();
            handle_command();

            generate(l2, "NULL ", -1, -1);
        } else {
            generate(l1, "NULL ", -1, -1);
        }
    } else {
        print_sintax_error("entao");
    }
}

// <etapa de declaração de sub-rotinas> ::= (<declaração de procedimento>;|
//                                          <declaração de função>;)
//                                         {<declaração de procedimento>;|
//                                          <declaração de função>;}
bool handle_subroutine_section() {
    bool has_inner_subprogram = false;

    if (current_token.symbol == SPROCEDIMENTO || current_token.symbol == SFUNCAO) {
        generate(-1, "JMP  ", label, -1);
        label++;
        has_inner_subprogram = true;
    }

    // Loop to handle multiple procedure or function declarations.
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

// <declaração de procedimento> ::= procedimento <identificador>;
//                                 <bloco>
void handle_procedure_declaration() {
    int l1 = label++;
    SymbolNode * proc;

    get_next_token();

    if (current_token.symbol == SIDENTIFICADOR) {
        // Semantic action: Check if subroutine can be declared and insert into table.
        if (can_declare_subroutine(current_token.lexem)) {
            insert_node_table(current_token.lexem, true, UNDEFINED, PROCEDURE, -1);
            proc = table;

            generate(l1, "NULL", -1, -1);

            get_next_token();
            if (current_token.symbol == SPONTO_VIRGULA) {
                get_next_token();
                handle_block(false);
            } else {
                print_sintax_error(";");
            }
        }
        else {
            // Semantic error: Identifier already exists.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s já existe\n", current_line, current_token.lexem);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador do procedimento após 'procedimento'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    // Semantic action: Pop the current scope.
    pop_scope();
    proc->mem = l1; // Insert on mem the procedure label
    generate(-1, "RETURN", -1, -1);
}

// <declaração de função> ::= funcao <identificador>: <tipo>;
//                            <bloco>
void handle_function_declaration() {
    int l1 = label++;
    SymbolNode * func;

    get_next_token();
    if (current_token.symbol == SIDENTIFICADOR) {
        // Semantic action: Check if subroutine can be declared and insert into table.
        if (can_declare_subroutine(current_token.lexem)) {
            insert_node_table(current_token.lexem, true, UNDEFINED, FUNC, 0);
            func = table;

            generate(l1, "NULL ", -1, -1);
            
            get_next_token();
            if (current_token.symbol == SDOISPONTOS) {
                get_next_token();
                if (current_token.symbol == SINTEIRO || current_token.symbol == SBOOLEANO) {
                    // Semantic action: Assign return type to the function.
                    if (current_token.symbol == SINTEIRO)
                        insert_type(INT);
                    else
                        insert_type(BOOL);
                    
                    get_next_token();
                    if (current_token.symbol == SPONTO_VIRGULA) {
                        get_next_token();
                        handle_block(false);
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
            // Semantic error: Identifier already exists.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: %s já existe\n", current_line, current_token.lexem);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO SINTÁTICO na linha %d: esperado identificador da função após 'funcao'\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    // Semantic action: Pop the current scope.
    pop_scope();
    printf("\n\n%d\n\n", l1);
    func->mem = l1; // Insert on mem the function label
    generate(-1, "RETURN", -1, -1);
}

// Função auxiliar para validar tipos e processar segunda expressão
void process_relational_operator(DataType * type1) {
    if (*type1 != INT) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    get_next_token();
    DataType type2 = handle_simple_expression();
    if (type2 != INT) {
        fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

// <expressão>::= <expressão simples> [<operador relacional><expressão simples>]
DataType handle_expression() {
    DataType type1 = handle_simple_expression();
    
    // Check for relational operator.
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
    
    return type1;
}

// <expressão simples> ::= [ + | - ] <termo> {( + | - | ou) <termo> }
DataType handle_simple_expression() {
    DataType ref = UNDEFINED;

    // Check for optional unary sign (+ or -).
    if (current_token.symbol == SMAIS || current_token.symbol == SMENOS) {
        get_next_token();
        ref = INT; // Unary signs imply integer type.
    }

    DataType type = handle_term();

    // Type check for the first term.
    if (type == INT) {
        ref = INT;
    } else {
        if (ref == INT) {
            // Unary sign with non-integer term is a semantic error.
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        } else {
            ref = BOOL; // If no unary sign, the type is determined by the term (BOOL).
        }
    }

    Simbolo * operations = (Simbolo*)malloc(100 * sizeof(Simbolo));
    int i_op = 0;

    // Loop for additive/OR operations.
    while (current_token.symbol == SMAIS || current_token.symbol == SMENOS || current_token.symbol == SOU) {
        operations[i_op] = current_token.symbol;
        i_op++;

        // Semantic check: 'ou' operator with integer type is an error.
        if (current_token.symbol == SOU && ref == INT) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        // Semantic check: Additive operators (+, -) with boolean type is an error.
        else if ((current_token.symbol == SMAIS || current_token.symbol == SMENOS) && ref == BOOL) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        get_next_token();
        // Semantic check: subsequent term must match the expression type.
        if (handle_term() != ref) {
            fprintf(stderr, "ERRO SEMANTICO na linha %d: operação com tipos incompatíveis\n", current_line);
            fflush(stderr);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i <= i_op; i++) {
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
    return ref; // Return the type of the simple expression.
}

// <termo>::= <fator> {(* | div | e) <fator>}
DataType handle_term() {
    DataType type1 = handle_factor();
    Simbolo s = current_token.symbol;

    Simbolo * operations = (Simbolo*)malloc(100 * sizeof(Simbolo));
    int i_op = 0;

    // Loop for multiplicative/AND operations.
    while(current_token.symbol == SMULT || current_token.symbol == SDIV || current_token.symbol == SE) {
        operations[i_op] = current_token.symbol;
        i_op++;

        get_next_token();
        DataType type2 = handle_factor();
        
        // Semantic check: Multiplication/Division must use INT types.
        if (s == SMULT || s == SDIV) {
            if (type1 != INT || type2 != INT) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação numérica com booleano\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else
                return INT; // Result is INT.
        }
        // Semantic check: 'e' (AND) must use BOOL types.
        else {
            if (type1 != BOOL || type2 != BOOL) {
                fprintf(stderr, "ERRO SEMANTICO na linha %d: operação lógica com inteiro\n", current_line);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else
                return BOOL; // Result is BOOL.
        }
    }

    for (int i = 0; i <= i_op; i++) {
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

    return type1; // Return the type of the factor.
}

// <fator> ::= (<variável> |
//              <número> |
//              <chamada de função> |
//              (<expressão>) | verdadeiro | falso |
//              nao <fator>)
DataType handle_factor() {
    switch (current_token.symbol) {
        case SIDENTIFICADOR:
            // Semantic action: Check if symbol is a variable or function.
            {
                SymbolNode * node = get_symbol_from_lexem(current_token.lexem);
                if (node != NULL) {
                    if (node->structure_type == FUNC) { // Subroutine (function or procedure)
                        handle_function_call(node);
                    } 
                    else { // Variable
                        handle_variable(node);
                    }
                    return node->data_type; // Return variable or function return type.
                }
                else {
                    // Semantic error: Symbol does not exist.
                    fprintf(stderr, "ERRO SEMANTICO na linha %d: %s não existe\n", current_line, current_token.lexem);
                    fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case SNUMERO:
            generate(-1, "LDC  ", atoi(current_token.lexem), -1);
            get_next_token();
            return INT;
        case SVERDADEIRO:
            generate(-1, "LDC  ", 1, -1);
            get_next_token();
            return BOOL;
        case SFALSO:
            generate(-1, "LDC  ", 0, -1);
            get_next_token();
            return BOOL;
        case SNAO: // NOT operator
            get_next_token();
            // Semantic check: 'nao' must be used with BOOL type.
            if (handle_factor() == BOOL) {
                generate(-1, "NEG  ", -1, -1);
                return BOOL;
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
}

//<comando atribuicao>::= <identificador> := <expressão>
void handle_assignment_command(Token aux) {
    get_next_token();

    SymbolNode * obj_to_assign_value = get_symbol_from_lexem(aux.lexem);

    // Semantic check: type compatibility in assignment.
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

// <chamada de procedimento>::= <identificador>
void handle_procedure_call(Token aux) {
    SymbolNode * node = get_symbol_from_lexem(aux.lexem);
    if (node != NULL) {
        generate(-1, "CALL ", node->mem, -1);
    }
    else {
        // Semantic error: Identifier does not exist.
        fprintf(stderr, "ERRO SEMANTICO na linha %d: identificador %s não existe\n", current_line, aux.lexem);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

// <variável> ::= <identificador>
void handle_variable(SymbolNode * node) {
    generate(-1, "LDV  ", node->mem, -1);

    get_next_token();
}

// <chamada de função> ::= <identificador>
void handle_function_call(SymbolNode * node) {
    generate(-1, "CALL ", node->mem, -1);
    generate(-1, "LDV  ", 0, -1);

    get_next_token();
}