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
#include "./list/list.h"

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
	bool bit_presence : 1;
	bool is_read : 1;
	bool is_write : 1;
} VirtPageTable;

#define P_SIZE sysconf(_SC_PAGE_SIZE)

static struct
{
	Byte *ram;
	VirtPageTable *virt_table;
	Stack *ram_page;
	Stack *pagefile_piece;
	List **free_buffer;
	List *replace;
	size_t page_size;
	size_t buffer_count;
	size_t virt_count;
	int pagefile_descript;
	char pagefile_name[L_tmpnam];
} v_mem;

static VMem_void_ptr mask_b_m;
static VMem_size_t mask_m_p;
static unsigned shift_v_t, shift_b_m;

static void bit_map_revers(VMem_void_ptr, VMem_size_t);
static bool bit_map_check(VMem_void_ptr, VMem_size_t, bool);

static VMem_size_t buffer_size_complement(VMem_size_t);
static VMem_size_t buffer_size_round(VMem_size_t count);

static int buffer_call(VMem_void_ptr *, VMem_size_t);
static void buffer_return(VMem_void_ptr, VMem_size_t);

static int page_alloc(VMem_void_ptr *, size_t);
static size_t page_find(void);
static size_t page_find_filepage(void);
static size_t page_find_filepage_count(void);
static void page_get(size_t);
static void page_put(size_t);
static void page_swap(size_t, size_t);
static Byte *page_byte_prepare(VMem_void_ptr, bool);

static void bit_map_revers(VMem_void_ptr ptr, VMem_size_t count)
{
	for (register VMem_size_t i = 0u; i < count; i += mask_m_p + 1u)
	{
		register Byte *bit_map = v_mem.virt_table[(ptr + i) >> shift_v_t].bit_map;
		register size_t idx = ((ptr + i) & mask_b_m) >> shift_b_m;
		bit_map[idx / CHAR_BIT] ^= 1u << idx % CHAR_BIT;
	}
}

static bool bit_map_check(VMem_void_ptr ptr, VMem_size_t count, bool value)
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
		count = (VMem_size_t) powl(2.0l, ceill(log2l((long double) count)));
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

static int page_alloc(VMem_void_ptr * restrict ptr_to_ptr, size_t count)
{
	size_t idx = SIZE_MAX;
	for (register size_t i = 0u, j = 0u; i < v_mem.virt_count; ++i)
	{
		if (v_mem.virt_table[i].page_idx == SIZE_MAX && v_mem.virt_table[i].position == (off_t) -1)
		{
			++j;
			if (j == count)
			{
				idx = i + 1u - j;
				break;
			}
		}
		else
		{
			j = 0u;
		}
	}

	err_handler(idx, SIZE_MAX, err0);
	err_handler(stack_size(v_mem.ram_page) + stack_size(v_mem.pagefile_piece) + page_find_filepage_count() < count, 1, err0);

	register size_t i = 0u;
	for (; i < count && !stack_empty(v_mem.ram_page); ++i, ++idx)
	{
		int result_int;
		result_int = list_push_back(v_mem.replace, &idx);
		err_handler(result_int != STACK_SUCCESS, 1, err1);
		stack_top(v_mem.ram_page, &v_mem.virt_table[idx].page_idx);
		v_mem.virt_table[idx].bit_presence = true;
		stack_pop(v_mem.ram_page);
	}
	for (; i < count && !stack_empty(v_mem.pagefile_piece); ++i, ++idx)
	{
		stack_top(v_mem.pagefile_piece, &v_mem.virt_table[idx].position);
		stack_pop(v_mem.pagefile_piece);
	}
	for (; i < count; ++i, ++idx)
	{
		size_t free_idx = page_find_filepage();
		v_mem.virt_table[idx].position = v_mem.virt_table[free_idx].position;
		v_mem.virt_table[free_idx].position = (off_t) -1;
		v_mem.virt_table[free_idx].is_write = true;
	}

	*ptr_to_ptr = (idx - count) * v_mem.page_size;

	return 0;

err1:
	raise(SIGSEGV);

err0:
	return -1;
}

static void page_get(size_t virt_idx)
{
	int result_int;
	register VirtPageTable *cell = v_mem.virt_table + virt_idx;
	register Byte *map = mmap(
			NULL,
			v_mem.page_size,
			PROT_READ,
			MAP_SHARED,
			v_mem.pagefile_descript,
			cell->position / P_SIZE * P_SIZE
		);
	err_handler(map, MAP_FAILED, err0);

	memcpy(
			v_mem.ram + cell->page_idx * v_mem.page_size,
			map + cell->position % P_SIZE,
			v_mem.page_size
		);

	result_int = munmap(map, v_mem.page_size);
	err_handler(result_int, -1, err0);

	cell->bit_presence = true;
	result_int = list_push_back(v_mem.replace, &virt_idx);
	err_handler(result_int != LIST_SUCCESS, 1, err0);

	return;

err0:
	raise(SIGSEGV);
}

static void page_put(size_t virt_idx)
{
	int result_int;
	register VirtPageTable *cell = v_mem.virt_table + virt_idx;
	if (cell->is_write)
	{
		register Byte *map = mmap(
				NULL,
				v_mem.page_size,
				PROT_WRITE,
				MAP_SHARED,
				v_mem.pagefile_descript,
				cell->position / P_SIZE * P_SIZE
			);
		err_handler(map, MAP_FAILED, err0);

		memcpy(
				map + cell->position % P_SIZE,
				v_mem.ram + cell->page_idx * v_mem.page_size,
				v_mem.page_size
			);

		result_int = munmap(map, v_mem.page_size);
		err_handler(result_int, -1, err0);
	}

	cell->page_idx = SIZE_MAX;
	cell->bit_presence = false;
	cell->is_read = false;
	cell->is_write = false;
	list_pop_front(v_mem.replace);

	return;

err0:
	raise(SIGSEGV);
}

static void page_swap(size_t swap_out, size_t swap_in)
{
	int result_int;
	register VirtPageTable *cell_out = v_mem.virt_table + swap_out, *cell_in = v_mem.virt_table + swap_in;
	register Byte *map = mmap(
			NULL,
			v_mem.page_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			v_mem.pagefile_descript,
			cell_in->position / P_SIZE * P_SIZE
		);
	err_handler(map, MAP_FAILED, err0);

	for (register size_t i = 0u; i < v_mem.page_size; ++i)
	{
		Byte temp = map[i];
		map[i] = v_mem.ram[cell_out->page_idx * v_mem.page_size + i];
		v_mem.ram[cell_out->page_idx * v_mem.page_size + i] = temp;
	}

	result_int = munmap(map, v_mem.page_size);
	err_handler(result_int, -1, err0);

	cell_out->position = cell_in->position;
	cell_in->position = (off_t) -1;

	cell_in->page_idx = cell_out->page_idx;
	cell_out->page_idx = SIZE_MAX;

	cell_in->bit_presence = true;
	cell_out->bit_presence = false;
	cell_in->is_read = false;
	cell_out->is_read = false;
	cell_in->is_write = false;
	cell_out->is_write = false;

	list_pop_front(v_mem.replace);
	result_int = list_push_back(v_mem.replace, &swap_in);
	err_handler(result_int != LIST_SUCCESS, 1, err0);

	return;

err0:
	raise(SIGSEGV);
}

static size_t page_find(void)
{
	size_t idx;
	while (list_front(v_mem.replace, &idx), v_mem.virt_table[idx].is_read)
	{
		int result_int;

		v_mem.virt_table[idx].is_read = false;
		list_pop_front(v_mem.replace);
		result_int = list_push_back(v_mem.replace, &idx);
		err_handler(result_int != LIST_SUCCESS, 1, err0);
	}

	return idx;

err0:
	raise(SIGSEGV);
	return SIZE_MAX;
}

static size_t page_find_filepage(void)
{
	size_t first_approach = SIZE_MAX;
	for (ListIt it = list_it_begin(v_mem.replace), end = list_it_end(v_mem.replace); list_it_inequal(&it, &end); list_it_next(&it))
	{
		size_t idx = *(size_t *) list_it_data(&it);
		if (v_mem.virt_table[idx].position != (off_t) -1)
		{
			return idx;
		}
	}

	return first_approach;
}

static size_t page_find_filepage_count(void)
{
	size_t count = 0u;
	for (ListIt it = list_it_begin(v_mem.replace), end = list_it_end(v_mem.replace); list_it_inequal(&it, &end); list_it_next(&it))
	{
		size_t idx = *(size_t *) list_it_data(&it);
		if (v_mem.virt_table[idx].position != (off_t) -1)
		{
			++count;
		}
	}

	return count;
}

#define V_MEM_READ false
#define V_MEM_WRITE true

static Byte *page_byte_prepare(VMem_void_ptr ptr, bool rd_wr)
{
	bool is_correct = bit_map_check(ptr, 1u, true);
	err_handler(is_correct, false, err0);

	size_t virt_idx = ptr >> shift_v_t, idx_in_page = ptr & (mask_b_m | mask_m_p);
	if (!v_mem.virt_table[virt_idx].bit_presence)
	{
		if (stack_empty(v_mem.ram_page))
		{
			size_t idx = page_find();
			if (v_mem.virt_table[idx].position != (off_t) -1)
			{
				v_mem.virt_table[virt_idx].page_idx = v_mem.virt_table[idx].page_idx;
				page_put(idx);
				page_get(virt_idx);
			}
			else
			{
				page_swap(idx, virt_idx);
			}
		}
		else
		{
			stack_top(v_mem.ram_page, &v_mem.virt_table[virt_idx].page_idx);
			page_get(virt_idx);
		}
	}

	if (rd_wr == V_MEM_WRITE)
	{
		v_mem.virt_table[virt_idx].is_write = true;
	}
	v_mem.virt_table[virt_idx].is_read = true;

	return v_mem.ram + v_mem.virt_table[virt_idx].page_idx * v_mem.page_size + idx_in_page;

err0:
	raise(SIGSEGV);
	return V_MEM_NULL;
}

static int buffer_call(VMem_void_ptr * restrict ptr_to_ptr, VMem_size_t count)
{
	int result_int;
	if (count < v_mem.page_size)
	{
		VMem_size_t length = count;
		size_t i = v_mem.buffer_count - (size_t) log2l((long double) v_mem.page_size / count);
		for (; i < v_mem.buffer_count && list_empty(v_mem.free_buffer[i]); ++i, length *= 2u);
		if (i >= v_mem.buffer_count)
		{
			result_int = page_alloc(ptr_to_ptr, 1u);
			err_handler(result_int, -1, err0);
		}
		else
		{
			list_front(v_mem.free_buffer[i], ptr_to_ptr);
			list_pop_front(v_mem.free_buffer[i]);
		}
		length /= 2u;
		for (size_t j = i - 1u; length >= count; --j, length /= 2u)
		{
			VMem_void_ptr temp = *ptr_to_ptr + length;
			result_int = list_push_front(v_mem.free_buffer[j], &temp);
			err_handler(result_int != LIST_SUCCESS, 1, err1);
		}
	}
	else
	{
		result_int = page_alloc(ptr_to_ptr, count / v_mem.page_size);
		err_handler(result_int, -1, err0);
	}

	return 0;

err1:
	raise(SIGSEGV);

err0:
	return -1;
}

static void buffer_return(VMem_void_ptr ptr, VMem_size_t size)
{
	int result_int;

	for (bool is_free = true; size < v_mem.page_size && is_free; size *= 2u)
	{
		VMem_void_ptr buddy = ptr;
		if (ptr / size % 2u)
		{
			buddy -= size;
		}
		else
		{
			buddy += size;
		}

		is_free = bit_map_check(buddy, size, false);
		List *list = v_mem.free_buffer[(size_t) log2l((long double) size) - shift_b_m];
		if (is_free)
		{
			ListIt it = list_it_begin(list), end = list_it_end(list);
			for (; list_it_inequal(&it, &end) && *(VMem_void_ptr *) list_it_data(&it) != buddy; list_it_next(&it));
			list_it_erase(&it);
			if (ptr > buddy)
			{
				ptr -= size;
			}
		}
		else
		{
			list_push_front(list, &ptr);
			break;
		}
	}

	for (register VMem_size_t i = 0u; i + v_mem.page_size <= size; i += v_mem.page_size)
	{
		size_t idx = (ptr + i) >> shift_v_t;
		register VirtPageTable *cell = v_mem.virt_table + idx;
		if (cell->position != (off_t) -1)
		{
			result_int = stack_push(v_mem.pagefile_piece, &cell->position);
			err_handler(result_int != STACK_SUCCESS, 1, err0);
			cell->position = (off_t) -1;
		}
		if (cell->page_idx != SIZE_MAX)
		{
			result_int = stack_push(v_mem.ram_page, &cell->page_idx);
			err_handler(result_int != STACK_SUCCESS, 1, err0);
			cell->page_idx = SIZE_MAX;
		}
		if (cell->bit_presence)
		{
			ListIt it = list_it_begin(v_mem.replace), end = list_it_end(v_mem.replace);
			for (; list_it_inequal(&it, &end) && *(size_t *) list_it_data(&it) != idx; list_it_next(&it));
			list_it_erase(&it);
			cell->bit_presence = false;
		}
		cell->is_read = false;
		cell->is_write = false;
	}

	return;

err0:
	raise(SIGSEGV);
}

int v_mem_init(off_t pagefile_size, size_t ram_size, size_t page_size, size_t min_piece)
{
	err_handler(
			!page_size ||
			page_size & (page_size - 1u) ||
			COMPUTER_ARCHITECTURE <= (unsigned) log2l((long double) page_size) ||
			ram_size < page_size ||
			!min_piece ||
			min_piece & (min_piece - 1u) ||
			page_size % min_piece,
		1, err0);

	shift_v_t = (unsigned) log2l((long double) page_size);
	shift_b_m = (unsigned) log2l((long double) min_piece);
	mask_b_m = ~((VMem_void_ptr) 0u);
	mask_b_m = mask_b_m >> shift_b_m;
	mask_b_m = mask_b_m << (shift_b_m + COMPUTER_ARCHITECTURE - shift_v_t);
	mask_b_m = mask_b_m >> (COMPUTER_ARCHITECTURE - shift_v_t);
	mask_m_p = min_piece - 1u;

	size_t piece_in_page = page_size / min_piece, real_count = ram_size / page_size;
	pagefile_size -= pagefile_size % page_size;

	v_mem.page_size = page_size;
	v_mem.buffer_count = shift_v_t - shift_b_m;
	v_mem.virt_count = 1ull << (COMPUTER_ARCHITECTURE - shift_v_t);
	v_mem.ram = malloc(real_count * v_mem.page_size);
	err_handler(v_mem.ram, NULL, err0);

	v_mem.virt_table = malloc(v_mem.virt_count * sizeof(VirtPageTable));
	err_handler(v_mem.virt_table, NULL, err1);
	size_t i = 0u;
	for (; i < v_mem.virt_count; ++i)
	{
		v_mem.virt_table[i].position = (off_t) -1;
		v_mem.virt_table[i].page_idx = SIZE_MAX;
		v_mem.virt_table[i].bit_presence = false;
		v_mem.virt_table[i].is_read = false;
		v_mem.virt_table[i].is_write = false;
		v_mem.virt_table[i].bit_map = calloc((piece_in_page + CHAR_BIT - 1u) / CHAR_BIT, sizeof(Byte));
		err_handler(v_mem.virt_table, NULL, err2);
	}

	int result_int;
	size_t physical_page = 0u;
	result_int = stack_create(&v_mem.ram_page, sizeof(size_t));
	err_handler(result_int != STACK_SUCCESS, 1, err2);
	for (size_t page_idx = 0u; page_idx < real_count && physical_page < v_mem.virt_count - 1; ++page_idx, ++physical_page)
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
	result_int = list_create(&v_mem.replace, sizeof(size_t));
	err_handler(result_int != LIST_SUCCESS, 1, err4);

	result_int = stack_create(&v_mem.pagefile_piece, sizeof(off_t));
	err_handler(result_int != STACK_SUCCESS, 1, err5);
	for (off_t offset = 0; offset < pagefile_size && physical_page < v_mem.virt_count - 1; offset += (off_t) v_mem.page_size, ++physical_page)
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
	result_int = ftruncate(v_mem.pagefile_descript, pagefile_size);
	err_handler(result_int, -1, err7);

	return 0;

err7:
	close(v_mem.pagefile_descript);
	remove(v_mem.pagefile_name);
err6:
	stack_destroy(v_mem.pagefile_piece);
err5:
	list_destroy(v_mem.replace);
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

	list_destroy(v_mem.replace);

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

	VMem_size_t diff = align_count - count;
	VMem_void_ptr end = ptr + align_count;
	if (diff)
	{
		for (register size_t i = shift_v_t; i > shift_b_m; --i)
		{
			register VMem_size_t mask = ((VMem_size_t) 1u << (i - 1u));
			if (diff & mask)
			{
				end -= mask;
				result_int = list_push_front(v_mem.free_buffer[i - 1u - shift_b_m], &end);
				err_handler(result_int != LIST_SUCCESS, 1, err1);
			}
		}
	}

	return ptr + mask_m_p + 1u;

err1:
	raise(SIGSEGV);
err0:
	return V_MEM_NULL;
}

void v_mem_free(VMem_void_ptr ptr, VMem_size_t count)
{
	count = buffer_size_complement(count);
	err_handler(!count || count == V_MEM_SIZE_MAX, 1, err0);

	ptr -= mask_m_p + 1u;

	bool is_correct = bit_map_check(ptr, count, true);
	err_handler(is_correct, false, err0);

	bit_map_revers(ptr, count);

	if ((count & ~mask_b_m) != count)
	{
		for (size_t i = shift_b_m; i < shift_v_t; ++i)
		{
			VMem_size_t mask = (VMem_size_t) 1u << i;
			if (count & mask)
			{
				count -= mask;
				buffer_return(ptr + count, mask);
			}
		}
	}

	if (count)
	{
		buffer_return(ptr, count);
	}

	return;

err0:
	raise(SIGSEGV);
}

void v_mem_deref_l(VMem_void_ptr ptr, Byte value)
{
	err_handler(ptr < mask_m_p + 1u, 1, err0);
	ptr -= mask_m_p + 1u;

	Byte *result = page_byte_prepare(ptr, V_MEM_WRITE);

	*result = value;

	return;

err0:
	raise(SIGSEGV);
}

Byte v_mem_deref_r(VMem_void_ptr ptr)
{
	err_handler(ptr < mask_m_p + 1u, 1, err0);
	ptr -= mask_m_p + 1u;

	Byte *result = page_byte_prepare(ptr, V_MEM_READ);

	return *result;

err0:
	raise(SIGSEGV);
	return 0u;
}