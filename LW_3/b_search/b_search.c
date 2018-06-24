#include <stdint.h>
#include <stdlib.h>
#include "b_search.h"

#define _B_SEARCH_H_SAVE_

void *b_search(register const void *key, const void *base, size_t num, size_t size, int (compar)(const void *, const void *))
{
#	ifdef _B_SEARCH_H_SAVE_
	if (key == NULL || base == NULL || !size || compar == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (!num)
	{
		return NULL;
	}

	register const uint_least8_t *left = (const uint_least8_t *) base, *right = (const uint_least8_t *) base + size * (num - 1u);

	while (left < right)
	{
		register size_t midle = ((right - left) / size + 1u) / 2u * size;
		int cmp = compar(key, left + midle);
		if (cmp > 0)
		{
			left += midle;
		}
		else if (cmp < 0)
		{
			right -= midle;
		}
		else
		{
			return (void *) (left + midle);
		}
	}

	int cmp = compar(key, left);
	if (!cmp)
	{
		return (void *) left;
	}
	else
	{
		return NULL;
	}
}

void *lower_bound(register const void *key, const void *base, size_t num, size_t size, int (compar)(const void *, const void *))
{
#	ifdef _B_SEARCH_H_SAVE_
	if (key == NULL || base == NULL || !size || compar == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (!num)
	{
		return (void *) base;
	}

	register const uint_least8_t *left = (const uint_least8_t *) base, *right = (const uint_least8_t *) base + size * (num - 1u);

	while (left < right)
	{
		register size_t midle = ((right - left) / size + 1u) / 2u * size;
		int cmp = compar(key, left + midle);
		if (cmp > 0)
		{
			left += midle;
		}
		else
		{
			right -= midle;
		}
	}

	int cmp = compar(key, left);
	if (left == base && cmp <= 0)
	{
		return (void *) left;
	}

	return (void *) (left + size);
}

void *upper_bound(register const void *key, const void *base, size_t num, size_t size, int (compar)(const void *, const void *))
{
#	ifdef _B_SEARCH_H_SAVE_
	if (key == NULL || base == NULL || !size || compar == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (!num)
	{
		return (void *) base;
	}

	register const uint_least8_t *left = (const uint_least8_t *) base, *right = (const uint_least8_t *) base + size * (num - 1u);

	while (left < right)
	{
		register size_t midle = ((right - left) / size + 1u) / 2u * size;
		int cmp = compar(key, left + midle);
		if (cmp < 0)
		{
			right -= midle;
		}
		else
		{
			left += midle;
		}
	}

	int cmp = compar(key, left);
	if (left == base && cmp < 0)
	{
		return (void *) left;
	}

	return (void *) (left + size);
}