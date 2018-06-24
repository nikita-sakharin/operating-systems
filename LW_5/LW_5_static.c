#include <stdio.h>
#include <stdlib.h>

#include "./stack/stack.h"
#include "header.h"

int main(void)
{
	int return_value;

	Stack *stack;
	return_value = stack_create(&stack, sizeof(Type));
	err_handler(return_value != STACK_SUCCESS, 1, "stack_create()");

	for (return_value = 0; return_value != EOF;)
	{
		Type temp;
		return_value = scanf("%"SCN_TYPE, &temp);
		if (return_value == EOF)
		{
			break;
		}

		return_value = stack_push(stack, &temp);
		err_handler(return_value != STACK_SUCCESS, 1, "stack_push()");
	}
	putchar('\n');

	for (Type temp; !stack_empty(stack);)
	{
		return_value = stack_top(stack, &temp);
		err_handler(return_value, EOF, "stack_top()");

		return_value = stack_pop(stack);
		err_handler(return_value, EOF, "stack_pop()");

		return_value = printf("%"PRI_TYPE"\n", temp);
		err_handler(return_value, EOF, "printf()");
	}

	stack_destroy(stack);

	return 0;
}