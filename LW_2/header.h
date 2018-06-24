#ifndef _HEADER_H_
#define _HEADER_H_

#include <stdio.h>
#include <stdlib.h>

#define err_handler(curr_value, wrong_value, func_name_str)\
	if ((curr_value) == (wrong_value))\
	{\
		perror(func_name_str);\
		exit(EXIT_FAILURE);\
	}\

typedef int IntType;
#define PRI_IntType "d"

#endif