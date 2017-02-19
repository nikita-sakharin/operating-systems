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
	result = v_mem_init(65536, 32768u, 1024u, 32u);
	err_handler(result, -1, "v_mem_init");
	result = v_mem_deinit();
	err_handler(result, -1, "v_mem_deinit");

	return 0;
}
