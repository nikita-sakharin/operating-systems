#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tim_sort.h"

#define err_handler(curr_value, wrong_value, func_name_str)\
	if ((curr_value) == (wrong_value))\
	{\
		perror(func_name_str);\
		exit(EXIT_FAILURE);\
	}\

void str_fscan(FILE *in, char *str, size_t length);

#define BUFFER_SIZE 32768
#define BUFFER_SIZE_ "32767"

#define START_CAPACITY 64u

typedef char *Str;
int str_cmp(const void *s1, const void *s2);

int main(void)
{
	int return_value;

	size_t capacity = START_CAPACITY, count = 0u;
	Str *array = malloc(capacity * sizeof(Str));
	err_handler(array, NULL, "malloc()")

	char buffer[BUFFER_SIZE];

	for (return_value = 0; return_value != EOF;)
	{
		return_value = scanf("%"BUFFER_SIZE_"[^\n]%*1[\n]", buffer);

		if (return_value == EOF)
		{
			break;
		}

		if (count >= capacity)
		{
			capacity *= 2u;
			array = realloc(array, capacity * sizeof(Str));
			err_handler(array, NULL, "realloc()")
		}

		array[count] = malloc((strlen(buffer) + 1u) * sizeof(char));
		err_handler(array[count], NULL, "malloc()");
		strcpy(array[count], buffer);
		++count;
	}

	tim_sort(array, count, sizeof(char *), str_cmp);

	for (size_t i = 0u; i < count; ++i)
	{
		return_value = printf("%s\n", array[i]);
		err_handler(return_value, EOF, "fprintf()");
	}

	for (size_t i = 0u; i < count; ++i)
	{
		free(array[i]);
	}
	free(array);

	return 0;
}

int str_cmp(const void *s1, const void *s2)
{
	return strcmp(*(char **) s1, *(char **) s2);
}

#define FORMAT_LENGTH 80u

void str_fscan(FILE *in, char *str, size_t length)
{
	if (!length)
	{
		fprintf(stderr, "str_fscan(): length = %zu\n", length);
		exit(EXIT_FAILURE);
	}

	int return_value;

	char format[FORMAT_LENGTH];
	return_value = snprintf(format, FORMAT_LENGTH, "%%%zus", length - 1u);
	err_handler(return_value < 0, 1, "fscanf()");

	return_value = fscanf(in, format, str);
	err_handler(return_value, EOF, "fscanf()");
}