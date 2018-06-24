#ifndef _HEADER_H_
#define _HEADER_H_

#include <inttypes.h>

typedef int_least32_t Type;
#define SCN_TYPE SCNdLEAST32
#define PRI_TYPE PRIdLEAST32

#define err_handler(curr_value, wrong_value, func_name_str)\
	if ((curr_value) == (wrong_value))\
	{\
		perror(func_name_str);\
		exit(EXIT_FAILURE);\
	}\

#endif