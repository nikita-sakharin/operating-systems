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

//	result = v_mem_init(32768, 32768u, 1024u, 32u);
int main(void)
{
	int result;
	result = v_mem_init(1024, 2048u, 1024u, 32u);
	err_handler(result, -1, "v_mem_init()");

	VMem_size_t count0 = 1280u;
	VMem_void_ptr ptr0 = v_mem_alloc(count0);
	err_handler(ptr0, V_MEM_NULL, "v_mem_alloc()");

	VMem_size_t count1 = 1024u;
	VMem_void_ptr ptr1 = v_mem_alloc(count1);
	err_handler(ptr1, V_MEM_NULL, "v_mem_alloc()");

	v_mem_free(ptr0, count0);
	v_mem_free(ptr1, count1);

	result = v_mem_deinit();
	err_handler(result, -1, "v_mem_deinit()");

	printf("SUCCESS!\n");

	return 0;
}