#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

struct list_node
{
	ListNode *prev;
	ListNode *next;
	uint_least8_t data[];
};

struct list
{
	ListNode *head;
	ListNode *tail;
	size_t size;
	size_t count;
};

#define _LIST_H_SAVE_

int list_create(register List ** restrict list_ptr, size_t size)
{
#	ifdef _LIST_H_SAVE_
	if (list_ptr == NULL || !size)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register List *temp = aligned_alloc(alignof(List), sizeof(List));
	if (temp == NULL)
	{
		return LIST_NOMEM;
	}

	temp->head = temp->tail = NULL;
	temp->size = size;
	temp->count = 0u;

	*list_ptr = temp;

	return LIST_SUCCESS;
}

void list_destroy(register List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	for (register ListNode *temp = list->head, *local_temp; temp != NULL; temp = local_temp)
	{
		local_temp = temp->next;
		free(temp);
	}

	free(list);
}

void list_clear(register List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	for (register ListNode *temp = list->head, *local_temp; temp != NULL; temp = local_temp)
	{
		local_temp = temp->next;
		free(temp);
	}

	list->head = list->tail = NULL;
	list->count = 0u;
}

size_t list_sizeof(register const List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return list->size;
}

bool list_empty(register const List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return !list->count;
}

size_t list_size(register const List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return list->count;
}

int list_push_front(register List * restrict list, const void * restrict source)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL || source == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register ListNode *temp = aligned_alloc(alignof(ListNode), sizeof(ListNode) + list->size);
	if (temp == NULL)
	{
		return LIST_NOMEM;
	}

	temp->prev = NULL;
	temp->next = list->head;
	memcpy(temp->data, source, list->size);

	list->head = temp;
	if (list->tail == NULL)
	{
		list->tail = temp;
	}
	else
	{
		temp->next->prev = temp;
	}
	++list->count;

	return LIST_SUCCESS;
}

int list_push_back(register List * restrict list, const void * restrict source)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL || source == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register ListNode *temp = aligned_alloc(alignof(ListNode), sizeof(ListNode) + list->size);
	if (temp == NULL)
	{
		return LIST_NOMEM;
	}

	temp->prev = list->tail;
	temp->next = NULL;
	memcpy(temp->data, source, list->size);

	list->tail = temp;
	if (list->head == NULL)
	{
		list->head = temp;
	}
	else
	{
		temp->prev->next = temp;
	}
	++list->count;

	return LIST_SUCCESS;
}

int list_front(register const List * restrict list, void * restrict dest)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL || dest == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (list->head == NULL)
	{
		return LIST_EMPTY;
	}

	memcpy(dest, list->head->data, list->size);

	return LIST_SUCCESS;
}

void *list_front_data(register const List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (list->head == NULL)
	{
		return NULL;
	}

	return list->head->data;
}

int list_back(register const List * restrict list, void * restrict dest)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL || dest == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (list->tail == NULL)
	{
		return LIST_EMPTY;
	}

	memcpy(dest, list->tail->data, list->size);

	return LIST_SUCCESS;
}

void *list_back_data(register const List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (list->tail == NULL)
	{
		return NULL;
	}

	return list->tail->data;
}

int list_pop_front(register List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (list->head == NULL)
	{
		return LIST_EMPTY;
	}

	register ListNode *temp = list->head;

	list->head = temp->next;
	if (temp->next == NULL)
	{
		list->tail = NULL;
	}
	else
	{
		temp->next->prev = NULL;
	}
	--list->count;

	free(temp);

	return LIST_SUCCESS;
}

int list_pop_back(register List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (list->tail == NULL)
	{
		return LIST_EMPTY;
	}

	register ListNode *temp = list->tail;

	list->tail = temp->prev;
	if (temp->prev == NULL)
	{
		list->head = NULL;
	}
	else
	{
		temp->prev->next = NULL;
	}
	--list->count;

	free(temp);

	return LIST_SUCCESS;
}

void list_reverse(register List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register ListNode *temp = list->head, *local_temp;
	list->head = list->tail;
	list->tail = temp;
	for (; temp != NULL; temp = local_temp)
	{
		local_temp = temp->next;
		temp->next = temp->prev;
		temp->prev = local_temp;
	}
}

ListIt list_it_begin(register const List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	ListIt it = { .list = (List *) list, .node = list->head };

	return it;
}

ListIt list_it_end(register const List * restrict list)
{
#	ifdef _LIST_H_SAVE_
	if (list == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	ListIt it = { .list = (List *) list, .node = NULL };

	return it;
}

bool list_it_equal(register const ListIt *it1, register const ListIt *it2)
{
#	ifdef _LIST_H_SAVE_
	if (it1 == NULL || it2 == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (it1->node == NULL && it2->node == NULL)
	{
		return it1->list == it2->list;
	}

	return it1->node == it2->node;
}

bool list_it_inequal(register const ListIt *it1, register const ListIt *it2)
{
#	ifdef _LIST_H_SAVE_
	if (it1 == NULL || it2 == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (it1->node == NULL && it2->node == NULL)
	{
		return it1->list != it2->list;
	}

	return it1->node != it2->node;
}

int list_it_insert(register const ListIt * restrict it, const void * restrict source)
{
#	ifdef _LIST_H_SAVE_
	if (it == NULL || source == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register ListNode *temp = aligned_alloc(alignof(ListNode), sizeof(ListNode) + it->list->size);
	if (temp == NULL)
	{
		return LIST_NOMEM;
	}

	memcpy(temp->data, source, it->list->size);

	ListNode **node_ptr;
	if (it->node != NULL)
	{
		node_ptr = &it->node->prev;
	}
	else
	{
		node_ptr = &it->list->tail;
	}
	temp->prev = *node_ptr;
	temp->next = it->node;
	if (*node_ptr != NULL)
	{
		(*node_ptr)->next = temp;
	}
	*node_ptr = temp;
	if (it->list->head == it->node)
	{
		it->list->head = temp;
	}
	++it->list->count;

	return LIST_SUCCESS;
}

void list_it_erase(register const ListIt * restrict it)
{
#	ifdef _LIST_H_SAVE_
	if (it == NULL || it->node == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (it->node->prev != NULL)
	{
		it->node->prev->next = it->node->next;
	}
	else
	{
		it->list->head = it->node->next;
	}
	if (it->node->next != NULL)
	{
		it->node->next->prev = it->node->prev;
	}
	else
	{
		it->list->tail = it->node->prev;
	}
	--it->list->count;

	free(it->node);
}

void list_it_get(register const ListIt * restrict it, register void * restrict dest)
{
#	ifdef _LIST_H_SAVE_
	if (it == NULL || it->node == NULL || dest == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	memcpy(dest, it->node->data, it->list->size);
}

void list_it_set(register const ListIt * restrict it, register const void * restrict source)
{
#	ifdef _LIST_H_SAVE_
	if (it == NULL || it->node == NULL || source == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	memcpy(it->node->data, source, it->list->size);
}

void *list_it_data(register const ListIt * restrict it)
{
#	ifdef _LIST_H_SAVE_
	if (it == NULL || it->node == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return it->node->data;
}

void list_it_prev(register ListIt * restrict it)
{
#	ifdef _LIST_H_SAVE_
	if (it == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (it->node == NULL)
	{
		it->node = it->list->tail;
		return;
	}

	it->node = it->node->prev;
}

void list_it_next(register ListIt * restrict it)
{
#	ifdef _LIST_H_SAVE_
	if (it == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (it->node == NULL)
	{
		it->node = it->list->head;
		return;
	}

	it->node = it->node->next;
}