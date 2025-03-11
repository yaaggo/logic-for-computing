#ifndef EVAL_H
#define EVAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "validation.h"

#define MAX_VARS 52
#define MAX_EXPR 100

typedef enum { 
    PROPOSITION, 
    OPERATOR, 
    PARENTHESES 
} data_type;

typedef struct {
    data_type type;
    union {
        char prop;      // para proposições (A-Z, a-z)
        char operator;  // para operadores (~, &, |, ->, <->)
        char parentheses;
    } data;
} token;

void read_expression(const char *expression, token *elements, int *size, error_status *status);
int evaluate_expression(token *elements, int size, int values[MAX_VARS], error_status *status);
void generate_truth_table(const char *expression, int reverse_order);

#endif