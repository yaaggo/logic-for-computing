#include "eval.h"

// função que determina a precedência dos operadores lógicos
// retorna um valor numérico que representa a prioridade do operador
// quanto maior o valor, maior a precedência
static int precedence(char op) {
    switch (op) {
        case '~': return  4; // negação (NOT) - maior precedência
        case '&': return  3; // conjunção (AND)
        case '|': return  2; // disjunção (OR)
        case '-': return  1; // implicação (→)
        case '<': return  0; // bicondicional (↔) - menor precedência
        default:  return -1; // operador inválido
    }
}

// função que aplica um operador lógico aos operandos a e b
// implementa as operações básicas da lógica proposicional
static int apply_operator(int a, int b, char op) {
    switch (op) {
        case '~': return      !b; // negação: NOT b
        case '&': return  a && b; // conjunção: a AND b
        case '|': return  a || b; // disjunção: a OR b
        case '-': return !a || b; // implicação: NOT a OR b (equivalente a a → b)
        case '<': return  a == b; // bicondicional: a == b (equivalente a a ↔ b)
        default: return 0; // operador inválido, retorna falso
    }
}

// função que lê uma expressão lógica e a converte em tokens
// armazena os tokens no array 'elements' e atualiza o tamanho em 'size'
// verifica sintaxe básica e reporta erros via 'status'
void read_expression(const char *expression, token *elements, int *size, error_status *status) {
    int index = 0, len = strlen(expression);
    char prev_type = 0; // rastreia o tipo do elemento anterior para verificação de sintaxe
    
    for (int i = 0; i < len; i++) {
        if (isspace(expression[i])) continue; // ignora espaços em branco
        
        token elem; // token atual sendo processado
        
        if (isalpha(expression[i])) {
            // letras são interpretadas como proposições
            elem.type = PROPOSITION;
            elem.data.prop = expression[i];
            prev_type = 'p'; // marca que o último elemento foi uma proposição
        } else if (expression[i] == '~') {
            // operador de negação
            elem.type = OPERATOR;
            elem.data.operator = expression[i];
            prev_type = 'o'; // marca que o último elemento foi um operador
        } else if (strchr("&|", expression[i])) {
            // operadores AND e OR

            // verificação de sintaxe: operador binário precisa de operando à esquerda
            if (prev_type != 'p' && prev_type != ')') {
                set_error(status, ERR_MISSING_OPERAND, i, "Operador binario sem operando a esquerda");
                return;
            }
            elem.type = OPERATOR;
            elem.data.operator = expression[i];
            prev_type = 'o'; // marca que o último elemento foi um operador
        } else if (expression[i] == '(') {
            // parêntese de abertura
            elem.type = PARENTHESES;
            elem.data.parentheses = expression[i];
            prev_type = '('; // marca que o último elemento foi um parêntese de abertura
        } else if (expression[i] == ')') {
            // parêntese de fechamento
            elem.type = PARENTHESES;
            elem.data.parentheses = expression[i];
            prev_type = ')'; // marca que o último elemento foi um parêntese de fechamento
        } else if (expression[i] == '-' && i+1 < len && expression[i+1] == '>') {
            // operador de implicação (->)

            // verificação de sintaxe: operador binário precisa de operando à esquerda
            if (prev_type != 'p' && prev_type != ')') {
                set_error(status, ERR_MISSING_OPERAND, i, "Operador '->' sem operando a esquerda");
                return;
            }
            elem.type = OPERATOR;
            elem.data.operator = '-'; // usa '-' como representação interna de '->'
            i++; // avança um caractere adicional para pular o '>'
            prev_type = 'o'; // marca que o último elemento foi um operador
        } else if (expression[i] == '<' && i+2 < len && expression[i+1] == '-' && expression[i+2] == '>') {
            // operador de bicondicional (<->)

            // verificação de sintaxe: operador binário precisa de operando à esquerda
            if (prev_type != 'p' && prev_type != ')') {
                set_error(status, ERR_MISSING_OPERAND, i, "Operador '<->' sem operando a esquerda");
                return;
            }
            elem.type = OPERATOR;
            elem.data.operator = '<'; // usa '<' como representação interna de '<->'
            i += 2; // avança dois caracteres adicionais para pular o '->'
            prev_type = 'o'; // marca que o último elemento foi um operador
        } else {
            // caractere inválido encontrado
            char err_detail[50];
            sprintf(err_detail, "Encontrado '%c'", expression[i]);
            set_error(status, ERR_INVALID_SYMBOL, i, err_detail);
            return;
        }
        
        elements[index++] = elem; // adiciona o token ao array
        
        // verifica se a expressão não é muito longa
        if (index >= MAX_EXPR) {
            set_error(status, ERR_EXPRESSION_TOO_LONG, i, "");
            return;
        }
    }
    
    // verificação final: expressão não pode terminar com operador binário
    if (index > 0 && elements[index-1].type == OPERATOR && elements[index-1].data.operator != '~') {
        set_error(status, ERR_MISSING_OPERAND, len-1, "Operador binario sem operando a direita");
        return;
    }
    
    *size = index; // atualiza o tamanho da expressão
}

// função que avalia uma expressão lógica usando o algoritmo shunting yard
// recebe tokens, valores das proposições e retorna o resultado da avaliação
int evaluate_expression(token *elements, int size, int values[MAX_VARS], error_status *status) {
    int stack[MAX_EXPR], top = -1; // pilha de valores e seu topo
    char operators[MAX_EXPR]; // pilha de operadores
    int op_top = -1; // topo da pilha de operadores

    for (int i = 0; i < size; i++) {
        if (elements[i].type == PROPOSITION) {
            // se for uma proposição, empilha seu valor
            int idx;

            // calcula o índice no array de valores
            if (isupper(elements[i].data.prop)) {
                idx = elements[i].data.prop - 'A'; // letras maiúsculas: A=0, B=1, ...
            } else {
                idx = elements[i].data.prop - 'a' + 26; // letras minúsculas: a=26, b=27, ...
            }

            // verifica se o índice é válido
            if (idx < 0 || idx >= MAX_VARS) {
                set_error(status, ERR_INVALID_SYMBOL, i, "Proposicao fora do intervalo valido");
                return 0;
            }
            stack[++top] = values[idx]; // empilha o valor da proposição
        } else if (elements[i].type == OPERATOR) {
            char op = elements[i].data.operator;
            
            // caso especial para o operador de negação
            if (op == '~') {
                operators[++op_top] = op;
                continue;
            }
            
            // desempilha operadores com maior ou igual precedência
            while (op_top >= 0 && operators[op_top] != '(' && 
            (precedence(operators[op_top]) > precedence(op) ||
            (operators[op_top] != '-' && precedence(operators[op_top]) == precedence(op)))) {
                int b = stack[top--]; // operando direito
                if (operators[op_top] == '~') {
                    // operador unário (negação)
                    stack[++top] = !b;
                } else {
                    // operador binário
                    if (top < 0) {
                        set_error(status, ERR_MISSING_OPERAND, i, "Faltam operandos para o operador");
                        return 0;
                    }
                    int a = stack[top--]; // operando esquerdo
                    stack[++top] = apply_operator(a, b, operators[op_top]); // aplica o operador e empilha resultado
                }
                op_top--; // remove o operador da pilha
            }
            operators[++op_top] = op; // empilha o operador atual
        } else if (elements[i].data.parentheses == '(') {
            // parêntese de abertura: empilha diretamente
            operators[++op_top] = '(';
        } else if (elements[i].data.parentheses == ')') {
            // parêntese de fechamento: desempilha operadores até encontrar o parêntese de abertura
            while (op_top >= 0 && operators[op_top] != '(') {
                int b = stack[top--]; // operando direito
                if (operators[op_top] == '~') {
                    // operador unário (negação)
                    stack[++top] = !b;
                } else {
                    // operador binário
                    if (top < 0) {
                        set_error(status, ERR_MISSING_OPERAND, i, "Faltam operandos para o operador");
                        return 0;
                    }
                    int a = stack[top--]; // operando esquerdo
                    stack[++top] = apply_operator(a, b, operators[op_top]); // aplica o operador e empilha resultado
                }
                op_top--; // remove o operador da pilha
            }
            
            // verifica se foi encontrado o parêntese de abertura correspondente
            if (op_top < 0 || operators[op_top] != '(') {
                set_error(status, ERR_UNBALANCED_PARENTHESES, i, "Parentese de fechamento sem abertura correspondente");
                return 0;
            }
            
            op_top--; // remove o parêntese de abertura da pilha
        }
    }

    // processa os operadores restantes na pilha
    while (op_top >= 0) {
        int b = stack[top--]; // operando direito
        if (operators[op_top] == '~') {
            // operador unário (negação)
            stack[++top] = !b;
        } else {
            // operador binário
            if (top < 0) {
                set_error(status, ERR_MISSING_OPERAND, -1, "Faltam operandos para o operador");
                return 0;
            }
            int a = stack[top--]; // operando esquerdo
            stack[++top] = apply_operator(a, b, operators[op_top]); // aplica o operador e empilha resultado
        }
        op_top--; // remove o operador da pilha
    }

    // verifica se há exatamente um valor na pilha (resultado final)
    if (top != 0) {
        set_error(status, ERR_INVALID_EXPRESSION, -1, "Expressao desbalanceada - multiplos resultados na pilha");
        return 0;
    }

    return stack[top]; // retorna o resultado final
}

// função que gera uma tabela verdade para uma expressão lógica
// reverse_order controla se a tabela é gerada em ordem normal ou invertida
void generate_truth_table(const char *expression, int reverse_order) {
    token elements[MAX_EXPR]; // array para armazenar os tokens
    int size = 0; // tamanho da expressão em tokens
    error_status status; // status de erro
    
    init_error_status(&status); // inicializa o status de erro
    
    // valida a expressão antes de processar
    if (!validate_expression(expression, &status)) {
        fprintf(stderr, "Erro: %s\n", status.message);
        return;
    }
    
    // converte a expressão em tokens
    read_expression(expression, elements, &size, &status);
    if (status.code != SUCCESS) {
        fprintf(stderr, "Erro: %s\n", status.message);
        return;
    }
    
    // verifica se a expressão não está vazia
    if (size == 0) {
        fprintf(stderr, "Erro: Expressao vazia apos processamento\n");
        return;
    }
    
    // identifica quais variáveis (proposições) são usadas na expressão
    int used_vars[52] = {0}; // rastreia quais proposições são usadas
    int num_vars = 0; // número total de proposições usadas
    for (int i = 0; i < size; i++) {
        if (elements[i].type == PROPOSITION) {
            int idx;
            if (isupper(elements[i].data.prop)) {
                idx = elements[i].data.prop - 'A'; // letras maiúsculas: A=0, B=1, ...
            } else {
                idx = elements[i].data.prop - 'a' + 26; // letras minúsculas: a=26, b=27, ...
            }
            
            // marca a proposição como usada se ainda não foi contada
            if (idx >= 0 && idx < 52 && !used_vars[idx]) {
                used_vars[idx] = 1;
                num_vars++;
            }
        }
    }

    // verifica se existem proposições na expressão
    if (num_vars == 0) {
        fprintf(stderr, "Erro: Nenhuma proposicao valida encontrada na expressao\n");
        return;
    }

    // cria uma lista ordenada das proposições usadas
    char var_list[MAX_VARS];
    int index = 0;

    // adiciona letras maiúsculas primeiro
    for (int i = 0; i < 26; i++) {
        if (used_vars[i]) var_list[index++] = 'A' + i;
    }
    // depois adiciona letras minúsculas
    for (int i = 0; i < 26; i++) {
        if (used_vars[i+26]) var_list[index++] = 'a' + i;
    }
    
    int rows = (int) pow(2, num_vars); // número de linhas na tabela (2^num_vars)
    int values[MAX_VARS] = {0}; // array para armazenar os valores das proposições
    
    // imprime o cabeçalho da tabela
    for (int i = 0; i < num_vars; i++)
        printf(" %c |", var_list[i]);
    printf(" %s\n", expression);
    
    // imprime uma linha separadora
    for (int i = 0; i < num_vars * 4 + strlen(expression) + 2; i++) 
        printf("-");
    printf("\n");
    
    // determina a direção de iteração
    int step = reverse_order ? -1 : 1;
    int start = reverse_order ? rows - 1 : 0;
    int end = reverse_order ? -1 : rows;

    // gera cada linha da tabela verdade
    for (int i = start; i != end; i += step) {
        // calcula valores das proposições para esta linha
        for (int j = 0; j < num_vars; j++)
            if (isupper(var_list[j])) {
                values[var_list[j] - 'A'] = (i >> (num_vars - j - 1)) & 1;
            } else {
                values[var_list[j] - 'a' + 26] = (i >> (num_vars - j - 1)) & 1;
            }
        
        // imprime os valores das proposições
        for (int j = 0; j < num_vars; j++) {
            int idx;
            if (isupper(var_list[j])) {
                idx = var_list[j] - 'A';
            } else {
                idx = var_list[j] - 'a' + 26;
            }
            printf(" %c |", values[idx] ? 'V' : 'F'); // V para verdadeiro, F para falso
        }
        
        // avalia a expressão com os valores atuais
        init_error_status(&status);
        int result = evaluate_expression(elements, size, values, &status);
        
        // imprime o resultado ou uma mensagem de erro
        if (status.code != SUCCESS) {
            printf(" ERRO: %s\n", status.message);
        } else {
            printf(" %c\n", result ? 'V' : 'F'); // V para verdadeiro, F para falso
        }
    }
}