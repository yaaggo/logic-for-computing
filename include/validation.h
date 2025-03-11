#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_ERROR_MSG 200 ///< tamanho máximo da mensagem de erro


/// @brief códigos de erro para validação de expressões
typedef enum error_code{
    SUCCESS = 0,                ///< operação bem-sucedida
    ERR_INVALID_SYMBOL,         ///< símbolo inválido na expressão
    ERR_UNBALANCED_PARENTHESES, ///< parênteses desbalanceados
    ERR_INVALID_EXPRESSION,     ///< expressão inválida
    ERR_EMPTY_EXPRESSION,       ///< expressão vazia
    ERR_EXPRESSION_TOO_LONG,    ///< expressão excede o tamanho máximo permitido
    ERR_CONSECUTIVE_OPERATORS,  ///< operadores consecutivos sem operandos entre eles
    ERR_MISSING_OPERAND,        ///< operador sem operando correspondente
    ERR_MEMORY_ALLOCATION       ///< erro na alocação de memória
} error_code;

/// @brief estrutura para armazenar informações sobre erros de validação
typedef struct {
    error_code code;             ///< código do erro ocorrido
    char message[MAX_ERROR_MSG]; ///< mensagem descritiva do erro
    int position;                ///< posição do erro na expressão
} error_status;

/**
 * @brief inicializa a estrutura de status de erro
 * 
 * @param status ponteiro para a estrutura de erro a ser inicializada
 */
void init_error_status(error_status *status);

/**
 * @brief define um erro na estrutura de status
 * 
 * @param status ponteiro para a estrutura de erro
 * @param code código do erro ocorrido
 * @param position posição do erro na expressão
 * @param custom_msg mensagem personalizada para o erro
 */
void set_error(error_status *status, error_code code, 
               int position, const char *custom_msg);

/**
 * @brief verifica se os parênteses estão balanceados na expressão
 * 
 * @param expression string contendo a expressão lógica
 * @param status ponteiro para a estrutura de erro
 * @return int retorna 1 se os parênteses estiverem balanceados, 0 caso contrário
 */
int check_parentheses_balance(const char *expression, error_status *status);

/**
 * @brief verifica se há operadores consecutivos inválidos na expressão
 * 
 * @param expression string contendo a expressão lógica
 * @param status ponteiro para a estrutura de erro
 * @return int retorna 1 se a expressão for válida, 0 caso contrário
 */
int check_consecutive_operator(const char *expression, error_status *status);

/**
 * @brief valida se a expressão lógica segue as regras sintáticas corretas
 * 
 * @param expression string contendo a expressão lógica
 * @param status ponteiro para a estrutura de erro
 * @return int retorna 1 se a expressão for válida, 0 caso contrário
 */
int validate_expression(const char *expression, error_status *status);

#endif