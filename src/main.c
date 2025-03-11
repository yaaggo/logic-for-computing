#include "../include/eval.h"
#include "../include/validation.h"

int main() {
    char expression[MAX_EXPR];

    printf("digite uma expressao logica (use ~, &, |, ->, <->): ");
    fgets(expression, MAX_EXPR, stdin);
    
    size_t len = strlen(expression);
    
    if (len > 0 && expression[len-1] == '\n') {
        expression[len-1] = '\0';
    }
    
    generate_truth_table(expression, 0);
}