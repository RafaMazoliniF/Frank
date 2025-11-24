/**
 * @file semantic.c
 * @brief Implementação das funcionalidades de gerenciamento da Tabela de Símbolos e Análise Semântica.
 * * A Tabela de Símbolos é implementada como uma pilha de nós (SymbolNode) para suportar escopo aninhado.
 */
#include "semantic.h"
#include <string.h>

// ---------------------- Tabela de Símbolos ------------------------------
// É uma pilha implementada com listas encadeadas onde cada nó é uma linha da tabela.

/** Ponteiro para o topo da Tabela de Símbolos (pilha). */
SymbolNode * table = NULL;

/**
 * @brief Inicializa a Tabela de Símbolos, definindo o topo como NULL.
 */
void init_table() {
    table = NULL;
}

/**
 * @brief Cria e inicializa um novo nó para a Tabela de Símbolos.
 * * @param lexem O lexema (nome) do símbolo.
 * @param scope Flag indicando se este nó marca o início de um novo escopo (true para escopo de sub-rotina/programa).
 * @param data_type O tipo de dado do símbolo (INT, BOOL, UNDEFINED).
 * @param structure_type A estrutura do símbolo (VAR, FUNC, PROCEDURE, PROGRAM_NAME).
 * @param mem O endereço de memória ou rótulo (label) associado ao símbolo.
 * @return SymbolNode* O ponteiro para o novo nó alocado.
 */
SymbolNode * new_symbol_node(char * lexem, bool scope, DataType data_type, StructureType structure_type, unsigned int mem) {
    SymbolNode * node = (SymbolNode*)malloc(sizeof(SymbolNode));
    node->lexem = lexem;
    node->scope = scope;
    node->data_type = data_type;
    node->structure_type = structure_type;
    node->mem = mem;
    node->next = NULL;

    return node;
}

/**
 * @brief Insere um novo nó no topo da Tabela de Símbolos (operação PUSH).
 * * @param node O nó do símbolo a ser inserido.
 */
void push_symbol_node(SymbolNode * node) {
    node->next = table;
    table = node;
}

/**
 * @brief Cria um novo nó e o insere no topo da Tabela de Símbolos.
 * * @param lexem O lexema (nome) do símbolo.
 * @param scope Flag de escopo.
 * @param data_type O tipo de dado.
 * @param structure_type A estrutura do símbolo.
 * @param mem O endereço de memória/rótulo.
 */
void insert_node_table(char * lexem, bool scope, DataType data_type, StructureType structure_type, unsigned int mem) {
    SymbolNode * node = new_symbol_node(lexem, scope, data_type, structure_type, mem);
    push_symbol_node(node);
}

/**
 * @brief Remove o nó do topo da Tabela de Símbolos (operação POP).
 * * @return SymbolNode* O nó removido do topo.
 */
SymbolNode * pop_symbol_node() {
    SymbolNode * ret = table;
    table = table->next;

    return ret;
}

// ------------------- Análise Semântica ----------------------

/**
 * @brief Verifica se uma variável pode ser declarada no escopo atual.
 * * Uma variável só pode ser declarada se não houver outro identificador igual
 * declarado no mesmo escopo (até encontrar o início do escopo anterior).
 * * @param lexem O lexema da variável a ser declarada.
 * @return bool Retorna true se a declaração for permitida, false caso contrário.
 */
bool can_declare_variable(char * lexem) {
    SymbolNode * current = table;
    bool exited_scope = false;

    // Percorre a pilha até o início do escopo (ou o final da pilha)
    while (current != NULL && !exited_scope) {
        if (strcmp(current->lexem, lexem) == 0) {
            return false;
        }

        if (current->scope == true) {
            exited_scope = true;
        }
        current = current->next;
    }

    return true;
}

/**
 * @brief Verifica se uma sub-rotina (função ou procedimento) pode ser declarada.
 * * Uma sub-rotina só pode ser declarada se não houver outro identificador igual
 * declarado em nenhum escopo visível (globalmente único para sub-rotinas).
 * * @param lexem O lexema da sub-rotina a ser declarada.
 * @return bool Retorna true se a declaração for permitida, false caso contrário.
 */
bool can_declare_subroutine(char * lexem) {
    SymbolNode * current = table;

    // Percorre toda a pilha
    while (current != NULL) {
        if (strcmp(current->lexem, lexem) == 0){
            return false;
        }

        current = current ->next;
    }
    
    return true;
}

/**
 * @brief Busca um símbolo na Tabela de Símbolos a partir do seu lexema (nome).
 * * A busca é feita do topo para a base (do escopo mais interno para o mais externo).
 * * @param lexem O lexema do símbolo a ser procurado.
 * @return SymbolNode* O ponteiro para o nó do símbolo encontrado, ou NULL se não existir.
 */
SymbolNode * get_symbol_from_lexem(char * lexem) {
    for (SymbolNode * current = table; current != NULL; current = current->next) {
        if (strcmp(current->lexem, lexem) == 0) {
            return current;
        }
    }

    return NULL;
}

/**
 * @brief Atribui um tipo de dado a todos os símbolos recém-declarados (com tipo UNDEFINED).
 * * Útil após o reconhecimento do tipo na seção de declaração de variáveis ou funções.
 * A iteração para quando encontra um símbolo que não é VAR ou FUNC ou não possui tipo UNDEFINED.
 * * @param data_type O tipo (INT ou BOOL) a ser atribuído.
 */
void insert_type(DataType data_type) {
    for (SymbolNode * current = table; current->structure_type == VAR || current->structure_type == FUNC; current = current->next) {
        if (current->data_type == UNDEFINED) {
            current->data_type = data_type;
        }

        if (current->next == NULL) {
            break;
        }
    }
}

/**
 * @brief Remove da pilha todos os símbolos pertencentes ao escopo atual.
 * * O processo de remoção (POP) continua até que o topo da pilha seja igual ao nó `final`,
 * que marca o início do escopo a ser mantido (o escopo pai).
 * * @param final O nó que marca o início do escopo pai (que deve permanecer).
 */
void pop_scope(SymbolNode * final) {
    while (table != final && table != NULL) {
        pop_symbol_node();
    }
}