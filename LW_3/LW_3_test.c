#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tim_sort.h"
#include "./b_search/b_search.h"

void *mem_rand(void *data, size_t size);
/*
typedef int Type;
*/
typedef struct
{
	long long value;
	short key;
} Type;

int type_cmp(const void *s1, const void *s2);
bool type_is_sort(const Type *data, size_t count);
void type_print(const Type *data, size_t count);

void check(const Type *, const Type *, size_t, size_t);

int main(void)
{
	int result;

	srand((unsigned) time(NULL));

	size_t test_max_size, test_count;
	result = scanf("%zu%zu", &test_max_size, &test_count);
	if (result != 2)
	{
		perror("scanf()");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0u; i < test_count; ++i)
	{
		size_t count = rand() % test_max_size + 1u;

		Type *data1 = malloc(count * sizeof(Type)), *data2 = malloc(count * sizeof(Type));
		if (data1 == NULL || data2 == NULL)
		{
			perror("malloc()");
			exit(EXIT_FAILURE);
		}

		mem_rand(data1, count * sizeof(Type));
		memcpy(data2, data1, count * sizeof(Type));

		qsort(data1, count, sizeof(Type), type_cmp);
		tim_sort(data2, count, sizeof(Type), type_cmp);

		check(data1, data2, count, i);

		free(data1);
		free(data2);

		printf("test[%05zu], test_size = %zu\n", i, count);
	}

	return 0;
}

void check(const Type *data1, const Type *data2, size_t count, size_t idx)
{
	for (size_t i = 0u; i < count; ++i)
	{
		if (memcmp(data1 + i, data2 + i, sizeof(Type)))
		{
			fprintf(stderr, "ERROR: test[%zu]: i = [%zu]\n", idx, i);
			exit(EXIT_FAILURE);
		}
	}
}

void *mem_rand(register void *data, size_t size)
{
	for (register size_t i = 0u; i < size; ++i)
	{
		((unsigned char *) data)[i] = (unsigned char) rand();
	}

	return data;
}

int type_cmp(const void *s1, const void *s2)
{
//	return (*(Type *) s1 > *(Type *) s2) - (*(Type *) s1 < *(Type *) s2);
	return (((Type *) s1)->key > ((Type *) s2)->key) - (((Type *) s1)->key < ((Type *) s2)->key);
}

bool type_is_sort(const Type *data, size_t count)
{
	for (size_t i = 1u; i < count; ++i)
	{
		if (type_cmp(data + i - 1u, data + i) > 0)
		{
			printf("[%zu] > [%zu]\n", i - 1u, i);
			return false;
		}
	}

	return true;
}

void type_print(const Type *data, size_t count)
{
	for (size_t i = 0u; i < count; ++i)
	{
//		printf("[%02zu] = %d\n", i, data[i]);
		printf("[%02zu] = { .key = %hd, .value = %lld }\n", i, data[i].key, data[i].value);
	}
	putchar('\n');
}