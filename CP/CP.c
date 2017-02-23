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

int main(void)
{
	int result;
	result = v_mem_init(32768, 32768u, 1024u, 32u);
	err_handler(result, -1, "v_mem_init()");

	VMem_size_t count = 1280u;
	VMem_void_ptr ptr = v_mem_alloc(count);
	err_handler(ptr, V_MEM_NULL, "v_mem_alloc()");

	v_mem_free(ptr, count);

	result = v_mem_deinit();
	err_handler(result, -1, "v_mem_deinit()");

	printf("SUCCESS!\n");

	return 0;
}