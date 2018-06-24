#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>

#include "./stack/stack.h"
#include "header.h"

int main(void)
{
	int return_value;

	void *dl_lib = dlopen("libstack.so", RTLD_LAZY), *temp;
	err_handler(dl_lib, NULL, dlerror());

	int (*stack_create_ptr)(Stack **, size_t);
	temp = dlsym(dl_lib, "stack_create");
	err_handler(temp, NULL, dlerror());
	memcpy(&stack_create_ptr, &temp, sizeof(int (*)(Stack **, size_t)));

	void (*stack_destroy_ptr)(Stack *);
	temp = dlsym(dl_lib, "stack_destroy");
	err_handler(temp, NULL, dlerror());
	memcpy(&stack_destroy_ptr, &temp, sizeof(void (*)(Stack *)));

	bool (*stack_empty_ptr)(const Stack *);
	temp = dlsym(dl_lib, "stack_empty");
	err_handler(temp, NULL, dlerror());
	memcpy(&stack_empty_ptr, &temp, sizeof(bool (*)(const Stack *)));

	int (*stack_push_ptr)(Stack *, const void *);
	temp = dlsym(dl_lib, "stack_push");
	err_handler(temp, NULL, dlerror());
	memcpy(&stack_push_ptr, &temp, sizeof(int (*)(Stack *, const void *)));

	int (*stack_top_ptr)(const Stack *, void *);
	temp = dlsym(dl_lib, "stack_top");
	err_handler(temp, NULL, dlerror());
	memcpy(&stack_top_ptr, &temp, sizeof(int (*)(const Stack *, void *)));

	int (*stack_pop_ptr)(Stack *);
	temp = dlsym(dl_lib, "stack_pop");
	err_handler(temp, NULL, dlerror());
	memcpy(&stack_pop_ptr, &temp, sizeof(int (*)(Stack *)));

	Stack *stack;
	return_value = stack_create_ptr(&stack, sizeof(Type));
	err_handler(return_value != STACK_SUCCESS, 1, "stack_create()");

	for (return_value = 0; return_value != EOF;)
	{
		Type temp;
		return_value = scanf("%"SCN_TYPE, &temp);
		if (return_value == EOF)
		{
			break;
		}

		return_value = stack_push_ptr(stack, &temp);
		err_handler(return_value != STACK_SUCCESS, 1, "stack_push()");
	}
	putchar('\n');

	for (Type temp; !stack_empty_ptr(stack);)
	{
		return_value = stack_top_ptr(stack, &temp);
		err_handler(return_value, EOF, "stack_top()");

		return_value = stack_pop_ptr(stack);
		err_handler(return_value, EOF, "stack_pop()");

		return_value = printf("%"PRI_TYPE"\n", temp);
		err_handler(return_value, EOF, "printf()");
	}

	stack_destroy_ptr(stack);

	return_value = dlclose(dl_lib);
	err_handler(return_value != 0, 1, dlerror());

	return 0;
}