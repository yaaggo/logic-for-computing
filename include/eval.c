#include "eval.h"

static int precedence(char op) {
    switch (op) {
        case '~': return  4;
        case '&': return  3;
        case '|': return  2;
        case '-': return  1;
        case '<': return  0;
        default:  return -1;
    }
}

static int apply_operator(int a, int b, char op) {
    switch (op) {
        case '~': return      !b;
        case '&': return  a && b;
        case '|': return  a || b;
        case '-': return !a || b;
        case '<': return  a == b;
        default: return 0;
    }
}

void read_expression(const char *expression, token *elements, int *size, error_status *status) {
    int index = 0, len = strlen(expression);
    char prev_type = 0;
    
    for (int i = 0; i < len; i++) {
        if (isspace(expression[i])) continue;
        
        token elem;
        
        if (isalpha(expression[i])) {
            elem.type = PROPOSITION;
            elem.data.prop = expression[i];
            prev_type = 'p';
        } else if (expression[i] == '~') {
            elem.type = OPERATOR;
            elem.data.operator = expression[i];
            prev_type = 'o';
        } else if (strchr("&|", expression[i])) {

            if (prev_type != 'p' && prev_type != ')') {
                set_error(status, ERR_MISSING_OPERAND, i, "Operador binario sem operando a esquerda");
                return;
            }
            elem.type = OPERATOR;
            elem.data.operator = expression[i];
            prev_type = 'o';
        } else if (expression[i] == '(') {
            elem.type = PARENTHESES;
            elem.data.parentheses = expression[i];
            prev_type = '(';
        } else if (expression[i] == ')') {
            elem.type = PARENTHESES;
            elem.data.parentheses = expression[i];
            prev_type = ')';
        } else if (expression[i] == '-' && i+1 < len && expression[i+1] == '>') {

            if (prev_type != 'p' && prev_type != ')') {
                set_error(status, ERR_MISSING_OPERAND, i, "Operador '->' sem operando a esquerda");
                return;
            }
            elem.type = OPERATOR;
            elem.data.operator = '-';
            i++;
            prev_type = 'o';
        } else if (expression[i] == '<' && i+2 < len && expression[i+1] == '-' && expression[i+2] == '>') {

            if (prev_type != 'p' && prev_type != ')') {
                set_error(status, ERR_MISSING_OPERAND, i, "Operador '<->' sem operando a esquerda");
                return;
            }
            elem.type = OPERATOR;
            elem.data.operator = '<';
            i += 2;
            prev_type = 'o';
        } else {
            char err_detail[50];
            sprintf(err_detail, "Encontrado '%c'", expression[i]);
            set_error(status, ERR_INVALID_SYMBOL, i, err_detail);
            return;
        }
        
        elements[index++] = elem;
        
        if (index >= MAX_EXPR) {
            set_error(status, ERR_EXPRESSION_TOO_LONG, i, "");
            return;
        }
    }
    
    if (index > 0 && elements[index-1].type == OPERATOR && elements[index-1].data.operator != '~') {
        set_error(status, ERR_MISSING_OPERAND, len-1, "Operador binario sem operando a direita");
        return;
    }
    
    *size = index;
}

int evaluate_expression(token *elements, int size, int values[MAX_VARS], error_status *status) {
    int stack[MAX_EXPR], top = -1;
    char operators[MAX_EXPR];
    int op_top = -1;

    for (int i = 0; i < size; i++) {
        if (elements[i].type == PROPOSITION) {
            int idx;

            if (isupper(elements[i].data.prop)) {
                idx = elements[i].data.prop - 'A';
            } else {
                idx = elements[i].data.prop - 'a' + 26;
            }

            if (idx < 0 || idx >= MAX_VARS) {
                set_error(status, ERR_INVALID_SYMBOL, i, "Proposicao fora do intervalo valido");
                return 0;
            }
            stack[++top] = values[idx];
        } else if (elements[i].type == OPERATOR) {
            char op = elements[i].data.operator;
            
            if (op == '~') {
                operators[++op_top] = op;
                continue;
            }
            
            while (op_top >= 0 && operators[op_top] != '(' && 
            (precedence(operators[op_top]) > precedence(op) ||
            (operators[op_top] != '-' && precedence(operators[op_top]) == precedence(op)))) {
                int b = stack[top--];
                if (operators[op_top] == '~') {
                    stack[++top] = !b;
                } else {
                    if (top < 0) {
                        set_error(status, ERR_MISSING_OPERAND, i, "Faltam operandos para o operador");
                        return 0;
                    }
                    int a = stack[top--];
                    stack[++top] = apply_operator(a, b, operators[op_top]);
                }
                op_top--;
            }
            operators[++op_top] = op;
        } else if (elements[i].data.parentheses == '(') {
            operators[++op_top] = '(';
        } else if (elements[i].data.parentheses == ')') {
            while (op_top >= 0 && operators[op_top] != '(') {
                int b = stack[top--];
                if (operators[op_top] == '~') {
                    stack[++top] = !b;
                } else {
                    if (top < 0) {
                        set_error(status, ERR_MISSING_OPERAND, i, "Faltam operandos para o operador");
                        return 0;
                    }
                    int a = stack[top--];
                    stack[++top] = apply_operator(a, b, operators[op_top]);
                }
                op_top--;
            }
            
            if (op_top < 0 || operators[op_top] != '(') {
                set_error(status, ERR_UNBALANCED_PARENTHESES, i, "Parentese de fechamento sem abertura correspondente");
                return 0;
            }
            
            op_top--;
        }
    }

    while (op_top >= 0) {
        int b = stack[top--];
        if (operators[op_top] == '~') {
            stack[++top] = !b;
        } else {
            if (top < 0) {
                set_error(status, ERR_MISSING_OPERAND, -1, "Faltam operandos para o operador");
                return 0;
            }
            int a = stack[top--];
            stack[++top] = apply_operator(a, b, operators[op_top]);
        }
        op_top--;
    }

    if (top != 0) {
        set_error(status, ERR_INVALID_EXPRESSION, -1, "Expressao desbalanceada - multiplos resultados na pilha");
        return 0;
    }

    return stack[top];
}

void generate_truth_table(const char *expression, int reverse_order) {
    token elements[MAX_EXPR];
    int size = 0;
    error_status status;
    
    init_error_status(&status);
    
    if (!validate_expression(expression, &status)) {
        fprintf(stderr, "Erro: %s\n", status.message);
        return;
    }
    
    // Leitura da expressão
    read_expression(expression, elements, &size, &status);
    if (status.code != SUCCESS) {
        fprintf(stderr, "Erro: %s\n", status.message);
        return;
    }
    
    if (size == 0) {
        fprintf(stderr, "Erro: Expressao vazia apos processamento\n");
        return;
    }
    
    int used_vars[52] = {0};
    int num_vars = 0;
    for (int i = 0; i < size; i++) {
        if (elements[i].type == PROPOSITION) {
            int idx;
            if (isupper(elements[i].data.prop)) {
                idx = elements[i].data.prop - 'A';
            } else {
                idx = elements[i].data.prop - 'a' + 26;
            }
            
            if (idx >= 0 && idx < 52 && !used_vars[idx]) {
                used_vars[idx] = 1;
                num_vars++;
            }
        }
    }

    if (num_vars == 0) {
        fprintf(stderr, "Erro: Nenhuma proposicao valida encontrada na expressao\n");
        return;
    }

    char var_list[MAX_VARS];
    int index = 0;

    for (int i = 0; i < 26; i++) {
        if (used_vars[i]) var_list[index++] = 'A' + i;
    }
    for (int i = 0; i < 26; i++) {
        if (used_vars[i+26]) var_list[index++] = 'a' + i;
    }
    
    int rows = (int) pow(2, num_vars);
    int values[MAX_VARS] = {0};
    
    for (int i = 0; i < num_vars; i++)
        printf(" %c |", var_list[i]);
    printf(" %s\n", expression);
    
    for (int i = 0; i < num_vars * 4 + strlen(expression) + 2; i++) 
        printf("-");
    printf("\n");
    
    int step = reverse_order ? -1 : 1;
    int start = reverse_order ? rows - 1 : 0;
    int end = reverse_order ? -1 : rows;

    for (int i = start; i != end; i += step) {
        for (int j = 0; j < num_vars; j++)
            if (isupper(var_list[j])) {
                values[var_list[j] - 'A'] = (i >> (num_vars - j - 1)) & 1;
            } else {
                values[var_list[j] - 'a' + 26] = (i >> (num_vars - j - 1)) & 1;
            }
        
        for (int j = 0; j < num_vars; j++) {
            int idx;
            if (isupper(var_list[j])) {
                idx = var_list[j] - 'A';
            } else {
                idx = var_list[j] - 'a' + 26;
            }
            printf(" %c |", values[idx] ? 'V' : 'F');
        }
        
        init_error_status(&status);
        int result = evaluate_expression(elements, size, values, &status);
        
        if (status.code != SUCCESS) {
            printf(" ERRO: %s\n", status.message);
        } else {
            printf(" %c\n", result ? 'V' : 'F');
        }
    }
}