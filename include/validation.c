#include "validation.h"
#include "eval.h"

void init_error_status(error_status *status) {
    status->code = SUCCESS;        // código de erro inicial: sem erro
    status->message[0] = '\0';     // mensagem de erro vazia
    status->position = -1;         // posição inválida/não definida
}

void set_error(error_status *status, error_code code, int position, const char *custom_msg) {
    status->code = code;           // define o código de erro
    status->position = position;   // define a posição onde ocorreu o erro

    // configura a mensagem de erro apropriada baseada no código
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
    int balance = 0;  // contador de balanço: incrementa em '(', decrementa em ')'
    int i;
    
    // percorre a expressão caractere por caractere
    for (i = 0; expression[i]; i++) {
        if (expression[i] == '(') {
            balance++;  // encontrou parêntese de abertura
        } else if (expression[i] == ')') {
            balance--;  // encontrou parêntese de fechamento
            // se o balanço for negativo, há um parêntese de fechamento sem abertura correspondente
            if (balance < 0) {
                set_error(status, ERR_UNBALANCED_PARENTHESES, i, "Parentese de fechamento sem abertura correspondente");
                return 0;
            }
        }
    }
    
    // se o balanço final for positivo, há parênteses de abertura sem fechamento
    if (balance > 0) {
        set_error(status, ERR_UNBALANCED_PARENTHESES, i-1, "Parentese de abertura sem fechamento correspondente");
        return 0;
    }
    
    return 1;  // parênteses estão balanceados
}

int check_consecutive_operators(const char *expression, error_status *status) {
    for (int i = 0; expression[i]; i++) {
        // verifica operadores binários & e |
        if (strchr("&|", expression[i])) {
            // operador binário no início ou fim da expressão
            if (i == 0 || i == strlen(expression) - 1) {
                set_error(status, ERR_MISSING_OPERAND, i, "Operador binario sem operandos suficientes");
                return 0;
            }
            
            // operadores binários consecutivos
            if (strchr("&|", expression[i-1]) || strchr("&|", expression[i+1])) {
                char err_msg[50];
                sprintf(err_msg, "Operadores '%c' e '%c' consecutivos", expression[i-1], expression[i+1]);
                set_error(status, ERR_CONSECUTIVE_OPERATORS, i, err_msg);
                return 0;
            }
        }
    }
    return 1;  // não há operadores consecutivos inválidos
}


int validate_expression(const char *expression, error_status *status) {
    // verifica se a expressão é nula ou vazia
    if (!expression || strlen(expression) == 0) {
        set_error(status, ERR_EMPTY_EXPRESSION, 0, "");
        return 0;
    }
    
    // verifica se a expressão excede o tamanho máximo permitido
    if (strlen(expression) > MAX_EXPR) {
        set_error(status, ERR_EXPRESSION_TOO_LONG, 0, "");
        return 0;
    }
    
    // verifica o balanceamento de parênteses
    if (!check_parentheses_balance(expression, status)) {
        return 0;
    }
    
    // verifica operadores consecutivos inválidos
    if (!check_consecutive_operators(expression, status)) {
        return 0;
    }
    
    return 1;  // expressão é válida
}