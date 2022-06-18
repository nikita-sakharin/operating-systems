#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

typedef struct stack_node StackNode;

struct stack_node
{
	StackNode *next;
	uint_least8_t data[];
};

struct stack
{
	StackNode *head;
	size_t size;
	size_t count;
};

#define _STACK_H_SAVE_

int stack_create(register Stack ** restrict stack_ptr, size_t size)
{
#	ifdef _STACK_H_SAVE_
	if (stack_ptr == NULL || !size)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register Stack *temp = aligned_alloc(alignof(Stack), sizeof(Stack));
	if (temp == NULL)
	{
		return STACK_NOMEM;
	}

	temp->head = NULL;
	temp->size = size;
	temp->count = 0u;

	*stack_ptr = temp;

	return STACK_SUCCESS;
}

void stack_destroy(register Stack * restrict stack)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	for (register StackNode *temp = stack->head, *local_temp; temp != NULL; temp = local_temp)
	{
		local_temp = temp->next;
		free(temp);
	}

	free(stack);
}

void stack_clear(register Stack * restrict stack)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	for (register StackNode *temp = stack->head, *local_temp; temp != NULL; temp = local_temp)
	{
		local_temp = temp->next;
		free(temp);
	}

	stack->head = NULL;
	stack->count = 0u;
}

size_t stack_sizeof(register const Stack * restrict stack)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return stack->size;
}

bool stack_empty(register const Stack * restrict stack)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return !stack->count;
}

size_t stack_size(register const Stack * restrict stack)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return stack->count;
}

int stack_push(register Stack * restrict stack, const void * restrict source)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL || source == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register StackNode *temp = aligned_alloc(alignof(StackNode), sizeof(StackNode) + stack->size);
	if (temp == NULL)
	{
		return STACK_NOMEM;
	}

	temp->next = stack->head;
	memcpy(temp->data, source, stack->size);

	stack->head = temp;
	++stack->count;

	return STACK_SUCCESS;
}

int stack_top(register const Stack * restrict stack, void * restrict dest)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL || dest == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (stack->head == NULL)
	{
		return STACK_EMPTY;
	}

	memcpy(dest, stack->head->data, stack->size);

	return STACK_SUCCESS;
}

void *stack_data(register const Stack * restrict stack)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (stack->head == NULL)
	{
		return NULL;
	}

	return stack->head->data;
}

int stack_pop(register Stack * restrict stack)
{
#	ifdef _STACK_H_SAVE_
	if (stack == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (stack->head == NULL)
	{
		return STACK_EMPTY;
	}

	register StackNode *temp = stack->head;

	stack->head = temp->next;
	--stack->count;

	free(temp);

	return STACK_SUCCESS;
}