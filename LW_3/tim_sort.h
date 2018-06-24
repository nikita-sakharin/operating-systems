#ifndef _TIM_SORT_H_
#define _TIM_SORT_H_

void tim_sort(void *data, size_t count, size_t size, int (*cmp)(const void *, const void *));

#endif