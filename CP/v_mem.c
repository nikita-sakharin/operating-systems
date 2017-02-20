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

static VMem_void_ptr v_t_mask, b_m_mask;
static VMem_size_t m_p_mask;

static VMem_void_ptr buffer_call(VMem_size_t);
static void buffer_return(VMem_void_ptr, VMem_size_t);

static VMem_size_t complement(VMem_size_t);
static size_t divide_small(VMem_size_t);
static size_t divide_large(VMem_size_t);

static int page_get(size_t);
//static int page_put(size_t);
static size_t page_find(void);
static Byte *page_byte_prepare(VMem_void_ptr);

static VMem_void_ptr buffer_call(VMem_size_t count)//size of one of buffer or multiple page_size
{
	if (count < v_mem.page_size)
	{
		VMem_size_t length = count;
		size_t i = (size_t) log2l((long double) v_mem.page_size / count);
		for (; i < v_mem.buffer_count && list_empty(v_mem.free_buffer[i]); ++i, length *= 2u);
		if (i >= v_mem.buffer_count)
		{
			for (size_t j = 0u; length > count && j < ; ++j, length /= 2u)
			{
				int result_int;
				VMem_void_ptr ptr;
				list_front();
			}
		}
		else
		{
		}
	}
	for (size_t i = 0u, j = 0u; i < v_mem.virt_count; ++i)
	{
		if (v_mem.virt_table[i].page_idx == SIZE_MAX)
		{
			++j;
			if (j == count / v_mem.page_size)
			{
				
			}
		}
		else
		{
			j = 0u;
		}
	}

	if ()
	{
			
	}
}

static void buffer_return(VMem_void_ptr, VMem_size_t)
{
}

static VMem_size_t complement(VMem_size_t count)
{
	if (count & m_p_mask)
	{
		err_handler(count > V_MEM_SIZE_MAX - m_p_mask, 1, err0);

		count += m_p_mask;
		count &= ~m_p_mask;
	}

	return count;

err0:
	return V_MEM_SIZE_MAX;
}

static VMem_void_ptr divide_small(VMem_size_t count)
{
}

static VMem_void_ptr divide_large(VMem_size_t count)
{
}

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
/*
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
*/
static size_t page_find(void)
{
	return 0u;
}

static Byte *page_byte_prepare(VMem_void_ptr ptr)
{
	VMem_void_ptr sig_bits = ptr & v_t_mask, l_s_bits = ptr & ~v_t_mask;
	if (!v_mem.virt_table[sig_bits].bit_map[l_s_bits & b_m_mask] || ptr == V_MEM_NULL)
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

	size_t log_page_size = (size_t) log2l((long double) page_size);/*piece_in_page, real_count, pagefile_size, min_piece*/

	v_mem.pagefile_size = pagefile_size - pagefile_size % page_size;
	v_mem.min_piece = min_piece;
	v_mem.page_size = page_size;
	v_mem.piece_in_page = v_mem.page_size / v_mem.min_piece;
	v_mem.buffer_count = (size_t) log2l((long double) page_size / min_piece);
	v_mem.real_count = ram_size / v_mem.page_size;
	v_mem.virt_count = 1ull << (COMPUTER_ARCHITECTURE - log_page_size);
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
	v_mem.virt_table[0u].bit_map[0u] = 0x01u;

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

	v_t_mask = 0u;
	v_t_mask = ~v_t_mask >> log_page_size << log_page_size;
	b_m_mask = ~(v_t_mask | (min_piece - 1u));
	m_p_mask = min_piece - 1u;

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
	count = complement(count);
	err_handler(!count || count == SIZE_MAX, 1, err0);

	VMem_void_ptr ptr;
	if (count <= v_mem.page_size)
	{
		ptr = divide_small(count);
	}
	else
	{
		ptr = divide_large(count);
	}
	err_handler(ptr, V_MEM_NULL, err0);

	return ptr;

err0:
	return V_MEM_NULL;
}

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
