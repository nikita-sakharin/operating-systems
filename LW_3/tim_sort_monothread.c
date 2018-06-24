#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "./b_search/b_search.h"
#include "./stack/stack.h"

#include "tim_sort.h"

#define max(A, B) ((A) > (B) ? (A) : (B))
#define min(A, B) ((A) < (B) ? (A) : (B))

static void *array_offset(const void *, size_t, size_t);
static ptrdiff_t array_ptrdiff(const void *, const void *, size_t);
static void array_assign(void *, size_t, size_t, size_t);
static int array_cmp(void *, size_t, size_t, size_t, int (*)(const void *, const void *));
static void array_swap(void *, size_t, size_t, size_t, void *);
static void array_revers(void *, size_t, size_t, void *);

static size_t get_minrun(size_t);
static ptrdiff_t get_run(void *, size_t, size_t, int (*)(const void *, const void *));

static void insertion_sort(void *, size_t, size_t, int (*)(const void *, const void *), size_t);
static void merge(void *, size_t, size_t, size_t, int (*)(const void *, const void *));

typedef struct
{
	void *data;
	size_t length;
} SubArray;

#define TIP_SIZE 3u

#define X 0u
#define Y 1u
#define Z 2u

void tim_sort(register void *data, size_t count, size_t size, register int (*cmp)(const void *, const void *))
{
#	ifdef _TIM_SORT_H_SAVE_
	if (data == NULL || !size || cmp == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (count < 2u)
	{
		return;
	}

	int result;

	const size_t minrun = get_minrun(count);
	void * const buffer = malloc(size);
	if (buffer == NULL)
	{
		exit(EXIT_FAILURE);
	}

	Stack *temp;
	result = stack_create(&temp, sizeof(SubArray));
	if (result != STACK_SUCCESS)
	{
		exit(EXIT_FAILURE);
	}

	for (register size_t i = 0u, shift; i < count; i += shift)
	{
		ptrdiff_t run = get_run(array_offset(data, i, size), count - i, size, cmp);
		if (run < 0)
		{
			run *= -1;
			array_revers(array_offset(data, i, size), (size_t) run, size, buffer);
		}
		shift = min(count - i, max((size_t) run, minrun));
		insertion_sort(array_offset(data, i, size), shift, size, cmp, (size_t) run);

		SubArray section = { .data = array_offset(data, i, size), .length = shift };
		stack_push(temp, &section);
	}

	Stack *stack;
	result = stack_create(&stack, sizeof(SubArray));
	if (result != STACK_SUCCESS)
	{
		exit(EXIT_FAILURE);
	}

	while(!stack_empty(temp))
	{
		SubArray tip[TIP_SIZE];

		stack_top(temp, tip);
		stack_pop(temp);

		register size_t i;
		for (i = 1u; !stack_empty(stack) && i < TIP_SIZE; ++i)
		{
			stack_top(stack, tip + i);
			stack_pop(stack);
		}

		if (i == 3u && tip[Z].length <= tip[Y].length + tip[X].length)
		{
			if (tip[X].length > tip[Z].length)
			{
				merge(tip[Y].data, tip[Y].length, tip[Z].length, size, cmp);
				tip[Y].length += tip[Z].length;
			}
			else
			{
				merge(tip[X].data, tip[X].length, tip[Y].length, size, cmp);
				tip[X].length += tip[Y].length;
				tip[Y] = tip[Z];
			}
			--i;
		}
		else if (i == 2u && tip[Y].length <= tip[X].length)
		{
			merge(tip[X].data, tip[X].length, tip[Y].length, size, cmp);
			tip[X].length += tip[Y].length;
			--i;
		}

		for (register size_t j = i; j; --j)
		{
			result = stack_push(stack, tip + j - 1u);
			if (result != STACK_SUCCESS)
			{
				exit(EXIT_FAILURE);
			}
		}
	}

	while (stack_size(stack) > 1u)
	{
		SubArray lower, upper;

		do
		{
			stack_top(stack, &upper);
			stack_pop(stack);

			if (!stack_empty(stack))
			{
				stack_top(stack, &lower);
				stack_pop(stack);

				merge(upper.data, upper.length, lower.length, size, cmp);
				upper.length += lower.length;
			}

			result = stack_push(temp, &upper);
			if (result != STACK_SUCCESS)
			{
				exit(EXIT_FAILURE);
			}
		} while (!stack_empty(stack));

		do
		{
			stack_top(temp, &upper);
			stack_pop(temp);

			if (!stack_empty(temp))
			{
				stack_top(temp, &lower);
				stack_pop(temp);

				merge(lower.data, lower.length, upper.length, size, cmp);
				lower.length += upper.length;
			}

			result = stack_push(stack, &lower);
			if (result != STACK_SUCCESS)
			{
				exit(EXIT_FAILURE);
			}
		} while (!stack_empty(temp));
	}

	stack_destroy(temp);
	stack_destroy(stack);

	free(buffer);
}

inline static void *array_offset(register const void *data, register size_t i, register size_t size)
{
	return (uint_least8_t *) data + i * size;
}

inline static ptrdiff_t array_ptrdiff(register const void *data1, register const void *data2, register size_t size)
{
	return ((uint_least8_t *) data1 - (uint_least8_t *) data2) / (ptrdiff_t) size;
}

inline static void array_assign(register void *data, register size_t idx1, register size_t idx2, register size_t size)
{
	memcpy(array_offset(data, idx1, size), array_offset(data, idx2, size), size);
}

inline static int array_cmp(register void *data, size_t idx1, size_t idx2, register size_t size, register int (*cmp)(const void *, const void *))
{
	return cmp(array_offset(data, idx1, size), array_offset(data, idx2, size));
}

inline static void array_swap(register void *data, size_t idx1, size_t idx2, register size_t size, register void *buffer)
{
	memcpy(buffer, array_offset(data, idx1, size), size);
	array_assign(data, idx1, idx2, size);
	memcpy(array_offset(data, idx2, size), buffer, size);
}

inline static void array_revers(register void *data, size_t count, size_t size, void *buffer)
{
	for (register size_t i = 0u; i < count / 2u; ++i)
	{
		array_swap(data, i, count - 1u - i, size, buffer);
	}
}

static size_t get_minrun(register size_t n)
{
	register size_t r = 0u;
	while (n >= 64u)
	{
		r |= n & 1u;
		n >>= 1u;
	}

	return n + r;
}

static ptrdiff_t get_run(register void *data, size_t count, size_t size, register int (*cmp)(const void *, const void *))
{
	if (count < 2u)
	{
		return (ptrdiff_t) count;
	}

	register const bool dir = array_cmp(data, 0, 1, size, cmp) <= 0;
	register ptrdiff_t run = 2;
	while (run < (ptrdiff_t) count && (array_cmp(data, run - 1, run, size, cmp) <= 0) == dir)
	{
		++run;
	}

	return dir ? run : -run;
}

static void insertion_sort(register void *data, size_t count, size_t size, register int (*cmp)(const void *, const void *), size_t already_sort)
{
	if (count < 2u)
	{
		return;
	}

	void * const temp = malloc(size);
	if (temp == NULL)
	{
		exit(EXIT_FAILURE);
	}

	for (register size_t i = already_sort; i < count; ++i)
	{
		for (register size_t j = i; j > 0 && array_cmp(data, j - 1u, j, size, cmp) > 0; --j)
		{
			array_swap(data, j - 1, j, size, temp);
		}
	}

	free(temp);
}

#define GALLOPING_MODE 7

static void merge(register void *data, size_t count1, size_t count2, size_t size, register int (*cmp)(const void *, const void *))
{
	register void * const temp = malloc(count1 * size);
	if (temp == NULL)
	{
		exit(EXIT_FAILURE);
	}
	memcpy(temp, data, count1 * size);

	register size_t i1 = 0u, i2 = 0;
	ptrdiff_t last_chunk = 0;
	while (i1 < count1 && i2 < count2)
	{
		if (cmp(array_offset(temp, i1, size), array_offset(data, count1 + i2, size)) > 0)
		{
			memcpy(array_offset(data, i1 + i2, size), array_offset(data, count1 + i2, size), size);
			++i2;
			last_chunk < 0 ? --last_chunk : (last_chunk = -1);
		}
		else
		{
			memcpy(array_offset(data, i1 + i2, size), array_offset(temp, i1, size), size);
			++i1;
			last_chunk > 0 ? ++last_chunk : (last_chunk = 1);
		}

		if (llabs(last_chunk) >= GALLOPING_MODE)
		{
			if (last_chunk > 0)
			{
				const void *position = upper_bound(array_offset(data, count1 + i2, size), array_offset(temp, i1, size), count1 - i1, size, cmp);
				const ptrdiff_t shift = array_ptrdiff(position, array_offset(temp, i1, size), size);
				memcpy(array_offset(data, i1 + i2, size), array_offset(temp, i1, size), shift * size);
				i1 += shift;
			}
			else
			{
				const void *position = lower_bound(array_offset(temp, i1, size), array_offset(data, count1 + i2, size), count2 - i2, size, cmp);
				const ptrdiff_t shift = array_ptrdiff(position, array_offset(data, count1 + i2, size), size);
				memmove(array_offset(data, i1 + i2, size), array_offset(data, count1 + i2, size), shift * size);
				i2 += shift;
			}

			last_chunk = 0;
		}
	}

	if (i1 < count1)
	{
		memcpy(array_offset(data, i1 + i2, size), array_offset(temp, i1, size), (count1 - i1) * size);
	}

	free(temp);
}