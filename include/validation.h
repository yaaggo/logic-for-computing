#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_ERROR_MSG 200

typedef enum {
    SUCCESS = 0,
    ERR_INVALID_SYMBOL,
    ERR_UNBALANCED_PARENTHESES,
    ERR_INVALID_EXPRESSION,
    ERR_EMPTY_EXPRESSION,
    ERR_EXPRESSION_TOO_LONG,
    ERR_CONSECUTIVE_OPERATORS,
    ERR_MISSING_OPERAND,
    ERR_MEMORY_ALLOCATION
} error_code;

typedef struct {
    error_code code;
    char message[MAX_ERROR_MSG];
    int position;
} error_status;

void init_error_status(error_status *status);

void set_error(error_status *status, error_code code, 
               int position, const char *custom_msg);

int check_parentheses_balance(const char *expression, error_status *status);
int check_consecutive_operator(const char *expression, error_status *status);
int validate_expression(const char *expression, error_status *status);

#endif