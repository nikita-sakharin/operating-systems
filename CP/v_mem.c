#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./list/list.h"
#include "./stack/stack.h"
#include "./queue/queue.h"

#include "v_mem.h"

#define err_handler(curr_value, wrong_value, mark)\
	if ((curr_value) == (wrong_value))\
	{\
		goto mark;\
	}

typedef struct
{
	off_t position;
	Byte *bit_map;
	size_t page_idx;
	bool bit_presence;
} VirtPageTable;

#define V_MEM_NULL ((VMem_void_ptr) 0)

#define P_SIZE sysconf(_SC_PAGE_SIZE)

static struct
{
	off_t pagefile_size;
	Byte *ram;
	VirtPageTable *virt_table;
	List **free_buffer;
	Stack *pagefile_piece;
	Stack *ram_page;
	Queue *page_replacement;
	size_t min_piece;
	size_t page_size;
	size_t piece_in_page;
	size_t buffer_count;
	size_t real_count;
	size_t virt_count;
	int pagefile_descript;
	char pagefile_name[L_tmpnam];
} v_mem;

static VMem_void_ptr mask_b_m;
static VMem_size_t mask_m_p;
static unsigned shift_v_t, shift_b_m;

static void bit_map_revers(VMem_void_ptr, VMem_size_t);
/*static */bool bit_map_check(VMem_void_ptr, VMem_size_t, bool);

static int page_virt_alloc(VMem_void_ptr *, size_t);
static void page_real_alloc(VMem_void_ptr, size_t);
static int buffer_call(VMem_void_ptr *, VMem_size_t);
//static void buffer_return(VMem_void_ptr, VMem_size_t);

static VMem_size_t buffer_size_complement(VMem_size_t);
static VMem_size_t buffer_size_round(VMem_size_t count);
/*
static int page_get(size_t);
static int page_put(size_t);
static Byte *page_byte_prepare(VMem_void_ptr);
*/
static void bit_map_revers(VMem_void_ptr ptr, VMem_size_t count)
{
	for (register VMem_size_t i = 0u; i < count; i += mask_m_p + 1u)
	{
		register Byte *bit_map = v_mem.virt_table[(ptr + i) >> shift_v_t].bit_map;
		register size_t idx = ((ptr + i) & mask_b_m) >> shift_b_m;
		bit_map[idx / CHAR_BIT] ^= 1u << idx % CHAR_BIT;
	}
}

/*static */bool bit_map_check(VMem_void_ptr ptr, VMem_size_t count, bool value)
{
	for (register VMem_size_t i = 0u; i < count; i += mask_m_p + 1u)
	{
		register Byte *bit_map = v_mem.virt_table[(ptr + i) >> shift_v_t].bit_map;
		register size_t idx = ((ptr + i) & mask_b_m) >> shift_b_m;
		register bool bit = bit_map[idx / CHAR_BIT] & (1u << idx % CHAR_BIT);
		if (value)
		{
			if (!bit)
			{
				return false;
			}
		}
		else
		{
			if (bit)
			{
				return false;
			}
		}
	}

	return true;
}

static int page_virt_alloc(VMem_void_ptr * restrict ptr_to_ptr, size_t count)
{
	err_handler(stack_size(v_mem.ram_page) + stack_size(v_mem.pagefile_piece) <= count, 1, err0);
	for (register size_t i = 0u, j = 0u; i < v_mem.virt_count; ++i)
	{
		if (v_mem.virt_table[i].page_idx == SIZE_MAX && v_mem.virt_table[i].position == (off_t) -1)
		{
			++j;
			if (j == count)
			{
				*ptr_to_ptr = (i - j) * v_mem.page_size;
				return 0;
			}
		}
		else
		{
			j = 0u;
		}
	}

err0:
	return -1;
}

static void page_real_alloc(VMem_void_ptr ptr, size_t count)
{
	size_t idx = ptr >> shift_v_t, i = 0u;
	for (; i < count && !stack_empty(v_mem.ram_page); ++i)
	{
		stack_top(v_mem.ram_page, &v_mem.virt_table[idx + i].page_idx);
		v_mem.virt_table[idx + i].bit_presence = true;
		stack_pop(v_mem.ram_page);
	}
	for (; i < count && !stack_empty(v_mem.pagefile_piece); ++i)
	{
		stack_top(v_mem.pagefile_piece, &v_mem.virt_table[idx + i].position);
		stack_pop(v_mem.pagefile_piece);
	}
}

static int buffer_call(VMem_void_ptr * restrict ptr_to_ptr, VMem_size_t count)//size of one of buffer or multiple page_size
{
	int result_int;
	size_t j, i;
	if (count < v_mem.page_size)
	{
		VMem_size_t length = count;
		i = v_mem.buffer_count - (size_t) log2l((long double) v_mem.page_size / count);
		for (; i < v_mem.buffer_count && list_empty(v_mem.free_buffer[i]); ++i, length *= 2u);
		if (i >= v_mem.buffer_count)
		{
			result_int = page_virt_alloc(ptr_to_ptr, 1u);
			err_handler(result_int, -1, err0);
		}
		else
		{
			list_front(v_mem.free_buffer[i], ptr_to_ptr);
		}
		for (j = i - 1u; length > count; --j, length /= 2u)
		{
			VMem_void_ptr temp = *ptr_to_ptr + length / 2u;
			result_int = list_push_front(v_mem.free_buffer[j], &temp);
			err_handler(result_int != LIST_SUCCESS, 1, err1);
		}
		if (i >= v_mem.buffer_count)
		{
			list_pop_front(v_mem.free_buffer[i]);
			page_real_alloc(*ptr_to_ptr, 1u);
		}
	}
	else
	{
		result_int = page_virt_alloc(ptr_to_ptr, count / v_mem.page_size);
		err_handler(result_int, -1, err0);
		page_real_alloc(*ptr_to_ptr, count / v_mem.page_size);
	}

	return 0;

err1:
	for (; j < i - 1u; ++j)
	{
		list_pop_front(v_mem.free_buffer[j + 1u]);
	}

err0:
	return -1;
}
/*
static void buffer_return(VMem_void_ptr, VMem_size_t)
{
}
*/
static VMem_size_t buffer_size_complement(VMem_size_t count)
{
	if (count & mask_m_p)
	{
		err_handler(count > V_MEM_SIZE_MAX - mask_m_p, 1, err0);

		count += mask_m_p;
		count &= ~mask_m_p;
	}

	return count;

err0:
	return V_MEM_SIZE_MAX;
}

static VMem_size_t buffer_size_round(VMem_size_t count)
{
	if (count < v_mem.page_size)
	{
		count = (VMem_size_t) powl(2.0l, ceil(log2l((long double) count)));
	}
	else if (count % v_mem.page_size)
	{
		err_handler(count > V_MEM_SIZE_MAX - (v_mem.page_size - count % v_mem.page_size), 1, err0);
		count += v_mem.page_size - count % v_mem.page_size;
	}

	return count;

err0:
	return V_MEM_SIZE_MAX;
}
/*
static int page_get(size_t virt_idx)
{
	Byte *map = mmap(
			NULL,
			v_mem.page_size,
			PROT_READ,
			MAP_SHARED,
			v_mem.pagefile_descript,
			v_mem.virt_table[virt_idx].position / P_SIZE * P_SIZE
		);
	if (map == MAP_FAILED)
	{
		return -1;
	}

	memcpy(
			v_mem.ram + v_mem.virt_table[virt_idx].page_idx * v_mem.page_size,
			map + v_mem.virt_table[virt_idx].position % P_SIZE,
			v_mem.page_size
		);
	v_mem.virt_table[virt_idx].bit_presence = true;

	return 0;
}

static int page_put(size_t virt_idx)
{
	Byte *map = mmap(
			NULL,
			v_mem.page_size,
			PROT_WRITE,
			MAP_SHARED,
			v_mem.pagefile_descript,
			v_mem.virt_table[virt_idx].position / P_SIZE * P_SIZE
		);
	err_handler(map, MAP_FAILED, err0);

	memcpy(
			map + v_mem.virt_table[virt_idx].position % P_SIZE,
			v_mem.ram + v_mem.virt_table[virt_idx].page_idx * v_mem.page_size,
			v_mem.page_size
		);
	v_mem.virt_table[virt_idx].page_idx = SIZE_MAX;
	v_mem.virt_table[virt_idx].bit_presence = false;

	return 0;

err0:
	return -1;
}

static Byte *page_byte_prepare(VMem_void_ptr ptr)
{
	VMem_void_ptr sig_bits = ptr & mask_v_t, l_s_bits = ptr & ~mask_v_t;
	if (!v_mem.virt_table[sig_bits].bit_map[l_s_bits & mask_b_m] || ptr == V_MEM_NULL)
	{
		raise(SIGSEGV);
	}
	else if (!v_mem.virt_table[sig_bits].bit_presence)
	{
		size_t page_idx = page_find();
		err_handler(page_idx, SIZE_MAX, err0);
		v_mem.virt_table[sig_bits].page_idx = page_idx;
		int result = page_get(sig_bits);
		err_handler(result, -1, err0);
	}

	return v_mem.ram + v_mem.virt_table[sig_bits].page_idx + l_s_bits;

err0:
	return NULL;
}
*/
int v_mem_init(off_t pagefile_size, size_t ram_size, size_t page_size, size_t min_piece)
{
	err_handler(
			!page_size ||
			page_size & (page_size - 1u) ||
			COMPUTER_ARCHITECTURE < (unsigned) log2l((long double) page_size) ||
			ram_size < page_size ||
			!min_piece ||
			min_piece & (min_piece - 1u) ||
			page_size % min_piece,
		1, err0);

	shift_v_t = (unsigned) log2l((long double) page_size);
	shift_b_m = (unsigned) log2l((long double) min_piece);/*piece_in_page, real_count, pagefile_size, min_piece*/
	mask_b_m = ~((VMem_void_ptr) 0u);
	mask_b_m = mask_b_m >> shift_b_m;
	mask_b_m = mask_b_m << (shift_b_m + COMPUTER_ARCHITECTURE - shift_v_t);
	mask_b_m = mask_b_m >> (COMPUTER_ARCHITECTURE - shift_v_t);
	mask_m_p = min_piece - 1u;

	v_mem.pagefile_size = pagefile_size - pagefile_size % page_size;
	v_mem.min_piece = min_piece;
	v_mem.page_size = page_size;
	v_mem.piece_in_page = v_mem.page_size / v_mem.min_piece;
	v_mem.buffer_count = shift_v_t - shift_b_m;
	v_mem.real_count = ram_size / v_mem.page_size;
	v_mem.virt_count = 1ull << (COMPUTER_ARCHITECTURE - shift_v_t);
	v_mem.ram = malloc(v_mem.real_count * v_mem.page_size);
	err_handler(v_mem.ram, NULL, err0);

	v_mem.virt_table = malloc(v_mem.virt_count * sizeof(VirtPageTable));
	err_handler(v_mem.virt_table, NULL, err1);
	size_t i = 0u;
	for (; i < v_mem.virt_count; ++i)
	{
		v_mem.virt_table[i].position = (off_t) -1;
		v_mem.virt_table[i].page_idx = SIZE_MAX;
		v_mem.virt_table[i].bit_presence = false;
		v_mem.virt_table[i].bit_map = calloc((v_mem.piece_in_page + CHAR_BIT - 1u) / CHAR_BIT, sizeof(Byte));
		err_handler(v_mem.virt_table, NULL, err2);
	}

	int result_int;
	result_int = stack_create(&v_mem.ram_page, sizeof(size_t));
	err_handler(result_int != STACK_SUCCESS, 1, err2);
	for (size_t page_idx = 0u; page_idx < v_mem.real_count; ++page_idx)
	{
		result_int = stack_push(v_mem.ram_page, &page_idx);
		err_handler(result_int != STACK_SUCCESS, 1, err3);
	}
	v_mem.free_buffer = malloc(v_mem.buffer_count * sizeof(List *));
	err_handler(v_mem.free_buffer, NULL, err3);
	size_t j = 0u;
	for (; j < v_mem.buffer_count; ++j)
	{
		result_int = list_create(v_mem.free_buffer + j, sizeof(VMem_void_ptr));
		err_handler(result_int != LIST_SUCCESS, 1, err4);
	}
	result_int = queue_create(&v_mem.page_replacement, sizeof(size_t));
	err_handler(result_int != QUEUE_SUCCESS, 1, err4);

	result_int = stack_create(&v_mem.pagefile_piece, sizeof(off_t));
	err_handler(result_int != STACK_SUCCESS, 1, err5);
	for (off_t offset = 0; offset < v_mem.pagefile_size; offset += (off_t) v_mem.page_size)
	{
		result_int = stack_push(v_mem.pagefile_piece, &offset);
		err_handler(result_int != STACK_SUCCESS, 1, err6);
	}
	static char *temp_name = NULL;
	if (temp_name == NULL)
	{
		temp_name = tmpnam(v_mem.pagefile_name);
	}
	err_handler(temp_name, NULL, err6);
	v_mem.pagefile_descript = open(temp_name, O_RDWR | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
	err_handler(v_mem.pagefile_descript, -1, err6);
	result_int = ftruncate(v_mem.pagefile_descript, v_mem.pagefile_size);
	err_handler(result_int, -1, err7);

	return 0;

err7:
	close(v_mem.pagefile_descript);
	remove(v_mem.pagefile_name);
err6:
	stack_destroy(v_mem.pagefile_piece);
err5:
	queue_destroy(v_mem.page_replacement);
err4:
	for (; j > 0u; --j)
	{
		list_destroy(v_mem.free_buffer[j - 1u]);
	}
	free(v_mem.free_buffer);
err3:
	stack_destroy(v_mem.ram_page);
err2:
	for (; i > 0u; --i)
	{
		free(v_mem.virt_table[i - 1u].bit_map);
	}
	free(v_mem.virt_table);
err1:
	free(v_mem.ram);
err0:
	return -1;
}

int v_mem_deinit(void)
{
	int return_value = 0, result_int;

	free(v_mem.ram);

	for (size_t i = 0u; i < v_mem.virt_count; ++i)
	{
		free(v_mem.virt_table[i].bit_map);
	}
	free(v_mem.virt_table);

	stack_destroy(v_mem.ram_page);

	for (size_t i = 0u; i < v_mem.buffer_count; ++i)
	{
		list_destroy(v_mem.free_buffer[i]);
	}
	free(v_mem.free_buffer);

	queue_destroy(v_mem.page_replacement);

	stack_destroy(v_mem.pagefile_piece);

	result_int = close(v_mem.pagefile_descript);
	if (result_int == -1)
	{
		return_value = result_int;
	}
	result_int = remove(v_mem.pagefile_name);
	if (result_int == EOF)
	{
		return_value = result_int;
	}
	
	return return_value;
}

VMem_void_ptr v_mem_alloc(VMem_size_t count)
{
	int result_int;

	count = buffer_size_complement(count);
	err_handler(!count || count == V_MEM_SIZE_MAX, 1, err0);
	VMem_size_t align_count = buffer_size_round(count);
	err_handler(align_count, V_MEM_SIZE_MAX, err0);

	VMem_void_ptr ptr;
	result_int = buffer_call(&ptr, align_count);
	err_handler(result_int, -1, err0);
	bit_map_revers(ptr, count);

	return ptr + mask_m_p + 1u;

err0:
	return V_MEM_NULL;
}
/*
void v_mem_free(VMem_void_ptr, VMem_size_t)
{
}

int v_mem_deref_l(VMem_void_ptr ptr, Byte *value)
{
	Byte *result = page_byte_prepare(ptr);
	if (result == NULL)
	{
		return -1;
	}

	*result = *value;

	return 0;
}

int v_mem_deref_r(VMem_void_ptr ptr, Byte *value)
{
	Byte *result = page_byte_prepare(ptr);
	if (result == NULL)
	{
		return -1;
	}

	*value = *result;

	return 0;
}
*/