#ifndef _STACK_H_
#define _STACK_H_

#include <stdbool.h>

#define STACK_SUCCESS 0
#define STACK_NOMEM 1
#define STACK_EMPTY 2

typedef struct stack Stack;

int stack_create(Stack **, size_t);

void stack_destroy(Stack *);

void stack_clear(Stack *);

size_t stack_sizeof(const Stack *);

bool stack_empty(const Stack *);

size_t stack_size(const Stack *);

int stack_push(Stack *, const void *);

int stack_top(const Stack *, void *);

int stack_pop(Stack *);

#endif