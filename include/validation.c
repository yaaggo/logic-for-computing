#include "validation.h"
#include "eval.h"

void init_error_status(error_status *status) {
    status->code = SUCCESS;
    status->message[0] = '\0';
    status->position = -1;
}

void set_error(error_status *status, error_code code, int position, const char *custom_msg) {
    status->code = code;
    status->position = position;

    switch (code) {
        case ERR_INVALID_SYMBOL:
            sprintf(status->message, "Simbolo invalido na expressao na posicao %d: %s", position, custom_msg);
            break;
        case ERR_UNBALANCED_PARENTHESES:
            sprintf(status->message, "Parenteses desbalanceados: %s", custom_msg);
            break;
        case ERR_INVALID_EXPRESSION:
            sprintf(status->message, "Expressao logica invalida: %s", custom_msg);
            break;
        case ERR_EMPTY_EXPRESSION:
            sprintf(status->message, "A expressao esta vazia");
            break;
        case ERR_EXPRESSION_TOO_LONG:
            sprintf(status->message, "A expressao excede o tamanho maximo permitido (%d caracteres)", MAX_EXPR); // mudar
            break;
        case ERR_CONSECUTIVE_OPERATORS:
            sprintf(status->message, "Operando consecutives invlidos na posicao %d: %s", position, custom_msg);
            break;
        case ERR_MISSING_OPERAND:
            sprintf(status->message, "Operando usente para o operador na posicao %d", position);
            break;
        case ERR_MEMORY_ALLOCATION:
            sprintf(status->message, "Falha de alocacao de memoria");
            break;
        default:
            sprintf(status->message, "Erro desocnhecido");
    }
}

int check_parentheses_balance(const char *expression, error_status *status) {
    int balance = 0;
    int i;
    
    for (i = 0; expression[i]; i++) {
        if (expression[i] == '(') {
            balance++;
        } else if (expression[i] == ')') {
            balance--;
            if (balance < 0) {
                set_error(status, ERR_UNBALANCED_PARENTHESES, i, "Parentese de fechamento sem abertura correspondente");
                return 0;
            }
        }
    }
    
    if (balance > 0) {
        set_error(status, ERR_UNBALANCED_PARENTHESES, i-1, "Parentese de abertura sem fechamento correspondente");
        return 0;
    }
    
    return 1;
}

int check_consecutive_operators(const char *expression, error_status *status) {
    for (int i = 0; expression[i]; i++) {
        if (strchr("&|", expression[i])) {
            if (i == 0 || i == strlen(expression) - 1) {
                set_error(status, ERR_MISSING_OPERAND, i, "Operador binario sem operandos suficientes");
                return 0;
            }
            
            if (strchr("&|", expression[i-1]) || strchr("&|", expression[i+1])) {
                char err_msg[50];
                sprintf(err_msg, "Operadores '%c' e '%c' consecutivos", expression[i-1], expression[i+1]);
                set_error(status, ERR_CONSECUTIVE_OPERATORS, i, err_msg);
                return 0;
            }
        }
    }
    return 1;
}

int validate_expression(const char *expression, error_status *status) {
    if (!expression || strlen(expression) == 0) {
        set_error(status, ERR_EMPTY_EXPRESSION, 0, "");
        return 0;
    }
    
    if (strlen(expression) > MAX_EXPR) {
        set_error(status, ERR_EXPRESSION_TOO_LONG, 0, "");
        return 0;
    }
    
    if (!check_parentheses_balance(expression, status)) {
        return 0;
    }
    
    if (!check_consecutive_operators(expression, status)) {
        return 0;
    }
    
    return 1;
}