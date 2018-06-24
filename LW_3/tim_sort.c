#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "./b_search/b_search.h"
#include "./stack/stack.h"

#include "tim_sort.h"

#include <pthread.h>

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
	pthread_t *thread_ptr;
	size_t thread_count;
} Sheaf;

static Stack *phase_1(void *, size_t, size_t, int (*)(const void *, const void *), Sheaf *);
static Stack *phase_2(Stack *, size_t, int (*)(const void *, const void *), Sheaf *);
static void phase_3(Stack *, size_t, int (*)(const void *, const void *));
static Stack *mutex_ptr(Stack *);

#define PHASE_COUNT 3u

#define FIRST 0u
#define SECOND 1u
#define THIRD 2u

typedef struct
{
	pthread_mutex_t *mutex_ptr;
	void *data;
	size_t length;
} SubArray;

static void *call_insertion_sort(void *);

typedef struct
{
	SubArray section;
	int (*cmp)(const void *, const void *);
	size_t size;
	size_t already_sort;
} InsertionSortArg;

static void *call_merge(void *);

typedef struct
{
	SubArray section[2u];
	int (*cmp)(const void *, const void *);
	size_t size;
} MergeArg;

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

	void *thread_result;
	Sheaf phase;
	Stack *mutex_ptr_stack;

	Stack *temp = phase_1(data, count, size, cmp, &phase);
	mutex_ptr_stack = mutex_ptr(temp);
	for (size_t i = 0u; i < phase.thread_count; ++i)
	{
		pthread_join(phase.thread_ptr[i], &thread_result);
		free(thread_result);
	}
	free(phase.thread_ptr);

	Stack *stack = phase_2(temp, size, cmp, &phase);
	stack_destroy(temp);
	for (size_t i = 0u; i < phase.thread_count; ++i)
	{
		pthread_join(phase.thread_ptr[i], &thread_result);
		free(thread_result);
	}
	free(phase.thread_ptr);

	phase_3(stack, size, cmp);
	stack_destroy(stack);

	while (!stack_empty(mutex_ptr_stack))
	{
		pthread_mutex_t *ptr;
		stack_top(mutex_ptr_stack, &ptr);
		stack_pop(mutex_ptr_stack);

		pthread_mutex_destroy(ptr);
		free(ptr);
	}

	stack_destroy(mutex_ptr_stack);
}

static Stack *phase_1(void * restrict data, const size_t count, const size_t size, int (*cmp)(const void *, const void *), Sheaf * restrict sheaf)
{
	int return_value;

	const size_t minrun = get_minrun(count);
	void * const buffer = malloc(size);
	if (buffer == NULL)
	{
		exit(EXIT_FAILURE);
	}

	Sheaf bunch = { .thread_ptr = malloc((count + minrun - 1u) / minrun * sizeof(pthread_t)), .thread_count = 0u };
	if (bunch.thread_ptr == NULL)
	{
		exit(EXIT_FAILURE);
	}

	Stack *result;
	return_value = stack_create(&result, sizeof(SubArray));
	if (return_value != STACK_SUCCESS)
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

		SubArray section = { .data = array_offset(data, i, size), .length = shift, .mutex_ptr = malloc(sizeof(pthread_mutex_t)) };
		if (section.mutex_ptr == NULL)
		{

			exit(EXIT_FAILURE);
		}
		return_value = pthread_mutex_init(section.mutex_ptr, NULL);
		if (return_value)
		{

			exit(EXIT_FAILURE);
		}
		return_value = stack_push(result, &section);
		if (return_value != STACK_SUCCESS)
		{

			exit(EXIT_FAILURE);
		}

		InsertionSortArg *arg_ptr = malloc(sizeof(InsertionSortArg));
		if (arg_ptr == NULL)
		{

			exit(EXIT_FAILURE);
		}
		arg_ptr->section = section;
		arg_ptr->cmp = cmp;
		arg_ptr->size = size;
		arg_ptr->already_sort = (size_t) run;

		return_value = pthread_create(bunch.thread_ptr + bunch.thread_count, NULL, call_insertion_sort, arg_ptr);
		if (return_value)
		{

			exit(EXIT_FAILURE);
		}
		++bunch.thread_count;
	}

	free(buffer);

	bunch.thread_ptr = realloc(bunch.thread_ptr, bunch.thread_count * sizeof(pthread_t));
	if (bunch.thread_ptr == NULL)
	{
		exit(EXIT_FAILURE);
	}

	*sheaf = bunch;

	return result;
}

#define TIP_SIZE 3u

#define X 0u
#define Y 1u
#define Z 2u

static Stack *phase_2(Stack *stack, size_t size, int (*cmp)(const void *, const void *), Sheaf * restrict sheaf)
{
	int return_value;

	Sheaf bunch = { .thread_ptr = malloc((stack_size(stack) - 1u) * sizeof(pthread_t)), .thread_count = 0u };
	if (bunch.thread_ptr == NULL)
	{
		exit(EXIT_FAILURE);
	}
	bool overlap = false;

	Stack *result;
	return_value = stack_create(&result, sizeof(SubArray));
	if (return_value != STACK_SUCCESS)
	{
		exit(EXIT_FAILURE);
	}

	while(!stack_empty(stack))
	{
		SubArray tip[TIP_SIZE];

		stack_top(stack, tip);
		stack_pop(stack);

		register size_t i;
		for (i = 1u; !stack_empty(result) && i < TIP_SIZE; ++i)
		{
			stack_top(result, tip + i);
			stack_pop(result);
		}

		if (i == 3u && tip[Z].length <= tip[Y].length + tip[X].length)
		{
			MergeArg *arg_ptr = malloc(sizeof(MergeArg));
			if (arg_ptr == NULL)
			{
				exit(EXIT_FAILURE);
			}
			arg_ptr->cmp = cmp;
			arg_ptr->size = size;

			if (tip[X].length > tip[Z].length)
			{
				arg_ptr->section[0u] = tip[Y];
				arg_ptr->section[1u] = tip[Z];

				tip[Y].length += tip[Z].length;
			}
			else
			{
				arg_ptr->section[0u] = tip[X];
				arg_ptr->section[1u] = tip[Y];

				tip[X].length += tip[Y].length;
				tip[Y] = tip[Z];
			}
			--i;

			if (overlap)
			{
				--bunch.thread_count;
				void *thread_result;
				pthread_join(bunch.thread_ptr[bunch.thread_count], &thread_result);
				free(thread_result);
			}
			overlap = true;

			return_value = pthread_create(bunch.thread_ptr + bunch.thread_count, NULL, call_merge, arg_ptr);
			if (return_value)
			{
				exit(EXIT_FAILURE);
			}
			++bunch.thread_count;
		}
		else if (i == 2u && tip[Y].length <= tip[X].length)
		{
			MergeArg *arg_ptr = malloc(sizeof(MergeArg));
			if (arg_ptr == NULL)
			{
				exit(EXIT_FAILURE);
			}
			arg_ptr->cmp = cmp;
			arg_ptr->size = size;

			arg_ptr->section[0u] = tip[X];
			arg_ptr->section[1u] = tip[Y];
			tip[X].length += tip[Y].length;
			--i;

			if (overlap)
			{
				--bunch.thread_count;
				void *thread_result;
				pthread_join(bunch.thread_ptr[bunch.thread_count], &thread_result);
				free(thread_result);
			}
			overlap = true;

			return_value = pthread_create(bunch.thread_ptr + bunch.thread_count, NULL, call_merge, arg_ptr);
			if (return_value)
			{
				exit(EXIT_FAILURE);
			}
			++bunch.thread_count;
		}
		else
		{
			overlap = false;
		}

		for (register size_t j = i; j; --j)
		{
			return_value = stack_push(result, tip + j - 1u);
			if (return_value != STACK_SUCCESS)
			{
				exit(EXIT_FAILURE);
			}
		}
	}

	if (bunch.thread_count)
	{
		bunch.thread_ptr = realloc(bunch.thread_ptr, bunch.thread_count * sizeof(pthread_t));
		if (bunch.thread_ptr == NULL)
		{
			exit(EXIT_FAILURE);
		}
	}

	*sheaf = bunch;

	return result;
}

static void phase_3(Stack *stack, size_t size, int (*cmp)(const void *, const void *))
{
	int return_value;

	Sheaf bunch = { .thread_ptr = malloc(stack_size(stack) / 2u * sizeof(pthread_t)), .thread_count = 0u };
	if (bunch.thread_ptr == NULL)
	{
		exit(EXIT_FAILURE);
	}

	Stack *temp;
	return_value = stack_create(&temp, sizeof(SubArray));
	if (return_value != STACK_SUCCESS)
	{
		exit(EXIT_FAILURE);
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

				MergeArg *arg_ptr = malloc(sizeof(MergeArg));
				if (arg_ptr == NULL)
				{
					exit(EXIT_FAILURE);
				}
				arg_ptr->cmp = cmp;
				arg_ptr->size = size;

				arg_ptr->section[0u] = upper;
				arg_ptr->section[1u] = lower;
				upper.length += lower.length;

				return_value = pthread_create(bunch.thread_ptr + bunch.thread_count, NULL, call_merge, arg_ptr);
				if (return_value)
				{
					exit(EXIT_FAILURE);
				}
				++bunch.thread_count;
			}

			return_value = stack_push(temp, &upper);
			if (return_value != STACK_SUCCESS)
			{
				exit(EXIT_FAILURE);
			}
		} while (!stack_empty(stack));
		for (size_t i = 0u; i < bunch.thread_count; ++i)
		{
			void *thread_result;
			pthread_join(bunch.thread_ptr[i], &thread_result);
			free(thread_result);
		}
		bunch.thread_count = 0u;

		do
		{
			stack_top(temp, &upper);
			stack_pop(temp);

			if (!stack_empty(temp))
			{
				stack_top(temp, &lower);
				stack_pop(temp);

				MergeArg *arg_ptr = malloc(sizeof(MergeArg));
				if (arg_ptr == NULL)
				{
					exit(EXIT_FAILURE);
				}
				arg_ptr->cmp = cmp;
				arg_ptr->size = size;

				arg_ptr->section[0u] = lower;
				arg_ptr->section[1u] = upper;
				lower.length += upper.length;

				return_value = pthread_create(bunch.thread_ptr + bunch.thread_count, NULL, call_merge, arg_ptr);
				if (return_value)
				{
					exit(EXIT_FAILURE);
				}
				++bunch.thread_count;
			}

			return_value = stack_push(stack, &lower);
			if (return_value != STACK_SUCCESS)
			{
				exit(EXIT_FAILURE);
			}
		} while (!stack_empty(temp));
	}
	for (size_t i = 0u; i < bunch.thread_count; ++i)
	{
		void *thread_result;
		pthread_join(bunch.thread_ptr[i], &thread_result);
		free(thread_result);
	}
	bunch.thread_count = 0u;

	free(bunch.thread_ptr);

	stack_destroy(temp);
}

static Stack *mutex_ptr(Stack * restrict stack)
{
	int return_value;

	Stack *result;
	return_value = stack_create(&result, sizeof(pthread_mutex_t *));
	if (return_value != STACK_SUCCESS)
	{
		exit(EXIT_FAILURE);
	}

	Stack *temp;
	return_value = stack_create(&temp, sizeof(SubArray));
	if (return_value != STACK_SUCCESS)
	{
		exit(EXIT_FAILURE);
	}

	SubArray buffer;

	while (!stack_empty(stack))
	{
		stack_top(stack, &buffer);
		stack_pop(stack);

		return_value = stack_push(result, &buffer.mutex_ptr);
		if (return_value != STACK_SUCCESS)
		{
			exit(EXIT_FAILURE);
		}

		return_value = stack_push(temp, &buffer);
		if (return_value != STACK_SUCCESS)
		{
			exit(EXIT_FAILURE);
		}
	}
	while (!stack_empty(temp))
	{
		SubArray buffer;

		stack_top(temp, &buffer);
		stack_pop(temp);

		return_value = stack_push(stack, &buffer);
		if (return_value != STACK_SUCCESS)
		{
			exit(EXIT_FAILURE);
		}
	}

	stack_destroy(temp);

	return result;
}

static void *call_insertion_sort(void *arguments)
{
	int return_value;
	InsertionSortArg arg = *(InsertionSortArg *) arguments;

	return_value = pthread_mutex_lock(arg.section.mutex_ptr);
	if (return_value)
	{
		exit(EXIT_FAILURE);
	}

	insertion_sort(arg.section.data, arg.section.length, arg.size, arg.cmp, arg.already_sort);

	return_value = pthread_mutex_unlock(arg.section.mutex_ptr);
	if (return_value)
	{
		exit(EXIT_FAILURE);
	}

	return arguments;
}

static void *call_merge(void *arguments)
{
	int return_value;
	MergeArg arg = *(MergeArg *) arguments;

	return_value = pthread_mutex_lock(arg.section[0u].mutex_ptr);
	if (return_value)
	{
		exit(EXIT_FAILURE);
	}
	return_value = pthread_mutex_lock(arg.section[1u].mutex_ptr);
	if (return_value)
	{
		exit(EXIT_FAILURE);
	}

	merge(arg.section[0u].data, arg.section[0u].length, arg.section[1u].length, arg.size, arg.cmp);

	return_value = pthread_mutex_unlock(arg.section[0u].mutex_ptr);
	if (return_value)
	{
		exit(EXIT_FAILURE);
	}
	return_value = pthread_mutex_unlock(arg.section[1u].mutex_ptr);
	if (return_value)
	{
		exit(EXIT_FAILURE);
	}

	return arguments;
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

static size_t get_minrun(register size_t counter)
{
	register size_t r = 0u;
	while (counter >= 64u)
	{
		r |= counter & 1u;
		counter >>= 1u;
	}

	return counter + r;
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