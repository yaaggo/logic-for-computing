#ifndef EVAL_H
#define EVAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "validation.h" // inclui funções de validação de expressões

// constantes para limites do programa
#define MAX_VARS 52  ///< número máximo de variáveis (a-z, A-Z = 52)
#define MAX_EXPR 100 ///< tamanho máximo da expressão em tokens

/// @brief tipos de dados que podem aparecer na expressão
typedef enum data_type{ 
    PROPOSITION,  ///< proposição lógica (variável como a, b, c, etc.)
    OPERATOR,     ///< operador lógico (~, &, |, ->, <->)
    PARENTHESES   ///< parênteses para controle de precedência
} data_type;

/// @brief estrutura para representar um token (elemento) da expressão
typedef struct {
    data_type type;  ///< tipo do token (proposição, operador ou parêntese)
    union data{
        char prop;        ///< para proposições (a-z, A-Z)
        char operator;    ///< para operadores (~, &, |, -, <)
        char parentheses; ///< para parênteses ('(' ou ')')
    } data;  ///< dados específicos do token
} token;

/**
 * @brief lê uma expressão lógica e a converte em tokens
 * 
 * @param expression string contendo a expressão lógica
 * @param elements array onde os tokens serão armazenados
 * @param size ponteiro para armazenar o número de tokens
 * @param status ponteiro para estrutura de status de erro
 */
void read_expression(const char *expression, token *elements, int *size, error_status *status);

/**
 * @brief avalia uma expressão lógica convertida em tokens
 * 
 * @param elements array de tokens da expressão
 * @param size número de tokens na expressão
 * @param values array com os valores booleanos das proposições
 * @param status ponteiro para estrutura de status de erro
 * @return int resultado booleano da expressão (0 = falso, 1 = verdadeiro)
 */
int evaluate_expression(token *elements, int size, int values[MAX_VARS], error_status *status);

/**
 * @brief gera uma tabela verdade para uma expressão lógica
 * 
 * @param expression string contendo a expressão lógica
 * @param reverse_order flag para controlar a ordem das linhas (0 = normal, 1 = invertida)
 */
void generate_truth_table(const char *expression, int reverse_order);

#endif // EVAL_H
