#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./list/list.h"

#include "v_mem.h"

#define err_handler(curr_value, wrong_value, func_name_str)\
	if ((curr_value) == (wrong_value))\
	{\
		perror(func_name_str);\
		exit(EXIT_FAILURE);\
	}

#define scanf_check(curr_value, wrong_value)\
	if ((curr_value) == (wrong_value))\
	{\
		break;\
	}

void menu_print(void);

int main(void)
{
	int result;

	result = v_mem_init(1024, 2048u, 1024u, 32u);
	err_handler(result, -1, "v_mem_init()");

	int code;
	menu_print();
	while (code = getchar(), code != EOF)
	{
		switch (code)
		{
			int result;
			case '\n':
			case ' ':
			{
				break;
			}
			case 'a':
			{
				printf("count: ");
				VMem_size_t count;
				result = scanf("%hu", &count);
				scanf_check(result, EOF);

				printf("count = %hu\n", count);
				VMem_void_ptr ptr = v_mem_alloc(count);
				printf("ptr = %#hx = %hu\n", ptr, ptr);

				putchar('\n');
				break;
			}
			case 'f':
			{
				printf("pointer: ");
				VMem_void_ptr ptr;
				result = scanf("%hu", &ptr);
				scanf_check(result, EOF);
				printf("pointer = %#hx = %hu\n", ptr, ptr);

				printf("count: ");
				VMem_size_t count;
				result = scanf("%hu", &count);
				scanf_check(result, EOF);
				printf("count = %hu\n", count);

				v_mem_free(ptr, count);

				putchar('\n');
				break;
			}
			case 'l':
			{
				printf("pointer: ");
				VMem_void_ptr ptr;
				result = scanf("%hu", &ptr);
				scanf_check(result, EOF);
				printf("pointer = %#hx = %hu\n", ptr, ptr);

				printf("value: ");
				Byte value;
				result = scanf("%hhu", &value);
				scanf_check(result, EOF);
				printf("value = %#hhx = %hhu\n", value, value);

				v_mem_deref_l(ptr, value);

				putchar('\n');
				break;
			}
			case 'r':
			{
				printf("pointer: ");
				VMem_void_ptr ptr;
				result = scanf("%hu", &ptr);
				scanf_check(result, EOF);
				printf("pointer = %#hx = %hu\n", ptr, ptr);

				Byte value = v_mem_deref_r(ptr);
				printf("value = %#hhx = %hhu\n", value, value);

				putchar('\n');
				break;
			}
			default:
			{
				puts("Wrong code");
				putchar('\n');
				break;
			}
		}
	}

	result = v_mem_deinit();
	err_handler(result, -1, "v_mem_deinit()");

	return 0;
}

void menu_print(void)
{
	puts("a - for alloc");
	puts("f - for free");
	puts("l - for set value");
	puts("r - for get value");
	putchar('\n');
}