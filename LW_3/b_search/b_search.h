#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#define _ALGORITHM_H_SAVE_

void *b_search(const void *key, const void *base, size_t num, size_t size, int (compar)(const void *, const void *));

void *lower_bound(const void *key, const void *base, size_t num, size_t size, int (compar)(const void *, const void *));

void *upper_bound(const void *key, const void *base, size_t num, size_t size, int (compar)(const void *, const void *));

void bubble_sort(void *data, size_t count, size_t size, int (*compar)(const void *, const void *));

void q_sort(void *data, size_t count, size_t size, int (*compar)(const void *, const void *));

#endif