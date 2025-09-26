#include "sintatic.h"

bool handle_simple_expression() {
    if (current_token.symbol != SMAIS && current_token.symbol != SMENOS && !handle_term()) {
        printf("Simple expression must initiate with \'+\', \'-\' or \"term\"\n");
        return false;
    } 

    if (current_token.symbol == SMAIS || current_token.symbol == SMENOS) {
        get_next_token();

        if (!handle_term()) {
            printf("Simple expression initiated with \'+\' or \'-\' must be followed by \"term\"\n");
            return false;
        }
    } 

    while (1) {
        get_next_token();
        if (current_token.symbol != SMAIS && current_token.symbol != SMENOS && current_token.symbol != SOU) {
            return true;
        } else {
            get_next_token();
            if (!handle_term()) {
                printf("Simple expression with multiple terms must be followed by \'+\', \'-\'or \'or\', then \"term\"\n");
                return false;
            }
        }
    }
}

bool handle_term() {
    if (!handle_factor()) {
        printf("Term must initiate with a valid \"factor\"\n");
        return false;
    } 

    while (1) {
        get_next_token();
        if (current_token.symbol != SMULT && current_token.symbol != SDIV && current_token.symbol != SE ) {
            return true;
        } else {
            get_next_token();
            if (!handle_factor()) {
                printf("Term with multiple \"factor\" must be followed by \'*\', \'div\' or \'e\', then a valid \"factor\"\n");
                return false;
            }
        }
    }
}

bool handle_factor() {
    if (handle_variable() ||
        handle_number() ||
        handle_function_call() ||
        current_token.symbol == SVERDADEIRO ||
        current_token.symbol == SFALSO
    ) {
        return true;
    }

    if (current_token.symbol == SABRE_PARENTESES) {
        get_next_token();
        if (!handle_expression()) {
            return false;
        } else {
            get_next_token();
            return current_token.symbol == SFECHA_PARENTESES;
        }
    }

    if (current_token.symbol = SNAO) {
        get_next_token();
        return handle_factor();
    }


    return false;
}

bool handle_variable() {
    if(current_token.symbol == SIDENTIFICADOR) {
        return isWordValid(current_token.lexem);
    }

    return false;
}

bool handle_function_call() {
    if(current_token.symbol == SIDENTIFICADOR) {
        return isWordValid(current_token.lexem);
    }

    return false;
}