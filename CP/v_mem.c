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

#include "./stack/stack.h"

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

typedef struct
{
	Byte *bit_map;
} RealPageTable;

#define V_MEM_NULL ((VMem_void_ptr) 0)

#define P_SIZE sysconf(_SC_PAGE_SIZE)

static struct
{
	off_t pagefile_size;
	Byte *ram;
	VirtPageTable *virt_table;
	RealPageTable *real_table;
	Stack *pagefile_piece;
	size_t min_piece;
	size_t page_size;
	size_t piece_in_page;
	size_t real_count;
	size_t virt_count;
	int pagefile_descript;
	char pagefile_name[L_tmpnam];
} v_mem;

static VMem_void_ptr v_t_mask, b_m_mask;

static int page_get(size_t);
static int page_put(size_t);

static size_t page_find(void);

static Byte *page_byte_prepare(VMem_void_ptr);

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

static size_t page_find(void)
{
	return page_idx;
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
	return NULL
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

	size_t log_page_size = (size_t) log2l((long double) page_size);

	v_mem.min_piece = min_piece;
	v_mem.page_size = page_size;
	v_mem.piece_in_page = v_mem.page_size / v_mem.min_piece;
	v_mem.real_count = ram_size / v_mem.page_size;
	v_mem.virt_count = 1ull << (COMPUTER_ARCHITECTURE - log_page_size);
	v_mem.ram = malloc(v_mem.real_count * v_mem.page_size);
	err_handler(v_mem.ram, NULL, err0);
	v_mem.real_table = malloc(v_mem.real_count * sizeof(RealPageTable));
	err_handler(v_mem.real_table, NULL, err1);
	size_t i = 0u;
	for (; i < v_mem.real_count; ++i)
	{
		v_mem.real_table[i].bit_map = calloc((v_mem.piece_in_page + CHAR_BIT - 1u) / CHAR_BIT, sizeof(Byte));
		err_handler(v_mem.real_table, NULL, err2);
	}
	v_mem.virt_table = malloc(v_mem.virt_count * sizeof(VirtPageTable));
	err_handler(v_mem.virt_table, NULL, err2);
	size_t j = 0u;
	for (; j < v_mem.virt_count; ++j)
	{
		v_mem.virt_table[j].position = (off_t) -1;
		v_mem.virt_table[j].page_idx = SIZE_MAX;
		v_mem.virt_table[j].bit_presence = false;
		v_mem.virt_table[j].bit_map = calloc((v_mem.piece_in_page + CHAR_BIT - 1u) / CHAR_BIT, sizeof(Byte));
		err_handler(v_mem.virt_table, NULL, err3);
	}
	v_mem.virt_table[0u].bit_map[0u] = 0x01u;

	int result_int;
	result_int = stack_create(&v_mem.pagefile_piece, sizeof(off_t));
	err_handler(result_int != STACK_SUCCESS, 1, err3);
	v_mem.pagefile_size = pagefile_size - pagefile_size % page_size;
	for (off_t offset = 0; offset < v_mem.pagefile_size; offset += (off_t) v_mem.page_size)
	{
		result_int = stack_push(v_mem.pagefile_piece, &offset);
		err_handler(result_int != STACK_SUCCESS, 1, err4);
	}
	static char *temp_name = NULL;
	if (temp_name == NULL)
	{
		temp_name = tmpnam(v_mem.pagefile_name);
	}
	err_handler(temp_name, NULL, err4);
	v_mem.pagefile_descript = open(temp_name, O_RDWR | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
	err_handler(v_mem.pagefile_descript, -1, err4);
	result_int = ftruncate(v_mem.pagefile_descript, v_mem.pagefile_size);
	err_handler(result_int, -1, err5);

	v_t_mask = 0u;
	v_t_mask = ~v_t_mask >> log_page_size << log_page_size;
	b_m_mask = ~(v_t_mask | (min_piece - 1u));

	return 0;

err5:
	close(v_mem.pagefile_descript);
	remove(v_mem.pagefile_name);
err4:
	stack_destroy(v_mem.pagefile_piece);
err3:
	for (; j > 0u; --j)
	{
		free(v_mem.virt_table[j - 1u].bit_map);
	}
	free(v_mem.virt_table);
err2:
	for (; i > 0u; --i)
	{
		free(v_mem.real_table[i - 1u].bit_map);
	}
	free(v_mem.real_table);
err1:
	free(v_mem.ram);
err0:
	return -1;
}

int v_mem_deinit(void)
{
	int return_value = 0, result_int;

	free(v_mem.ram);

	for (size_t i = 0u; i < v_mem.real_count; ++i)
	{
		free(v_mem.real_table[i].bit_map);
	}
	free(v_mem.real_table);

	for (size_t i = 0u; i < v_mem.virt_count; ++i)
	{
		free(v_mem.virt_table[i].bit_map);
	}
	free(v_mem.virt_table);

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

	stack_destroy(v_mem.pagefile_piece);
	
	return return_value;
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
