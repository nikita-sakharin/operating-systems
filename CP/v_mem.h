#ifndef _VIRT_MEM_H_
#define _VIRT_MEM_H_

#include <assert.h>
#include <stdint.h>

#define COMPUTER_ARCHITECTURE 16u

#if COMPUTER_ARCHITECTURE == 8
typedef uint_least8_t VMem_size_t;
#define V_MEM_SIZE_MAX UINT_LEAST8_MAX
typedef int_least8_t VMem_ptrdiff_t;
#define V_MEM_PTRDIFF_MIN INT_LEAST8_MIN
#define V_MEM_PTRDIFF_MAX INT_LEAST8_MAX
#elif COMPUTER_ARCHITECTURE == 16
typedef uint_least16_t VMem_size_t;
#define V_MEM_SIZE_MAX UINT_LEAST16_MAX
typedef int_least16_t VMem_ptrdiff_t;
#define V_MEM_PTRDIFF_MIN INT_LEAST16_MIN
#define V_MEM_PTRDIFF_MAX INT_LEAST16_MAX
#elif COMPUTER_ARCHITECTURE == 32
typedef uint_least32_t VMem_size_t;
#define V_MEM_SIZE_MAX UINT_LEAST32_MAX
typedef int_least32_t VMem_ptrdiff_t;
#define V_MEM_PTRDIFF_MIN INT_LEAST32_MIN
#define V_MEM_PTRDIFF_MAX INT_LEAST32_MAX
#elif COMPUTER_ARCHITECTURE == 64
typedef uint_least64_t VMem_size_t;
#define V_MEM_SIZE_MAX UINT_LEAST64_MAX
typedef int_least64_t VMem_ptrdiff_t;
#define V_MEM_PTRDIFF_MIN INT_LEAST64_MIN
#define V_MEM_PTRDIFF_MAX INT_LEAST64_MAX
#else
static_assert(0, "COMPUTER_ARCHITECTURE = "str_make(COMPUTER_ARCHITECTURE)" is invalid");
#endif
typedef VMem_size_t VMem_void_ptr;

typedef uint_least8_t Byte;

int v_mem_init(off_t, size_t, size_t, size_t);
int v_mem_deinit(void);

VMem_void_ptr v_mem_alloc(VMem_size_t);
void v_mem_free(VMem_void_ptr);

int v_mem_deref_l(VMem_void_ptr, Byte *);
int v_mem_deref_r(VMem_void_ptr, Byte *);

#endif