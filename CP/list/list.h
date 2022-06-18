#ifndef _LIST_H_
#define _LIST_H_

#include <stdbool.h>

#define LIST_SUCCESS 0
#define LIST_NOMEM 1
#define LIST_EMPTY 2

typedef struct list List;

typedef struct list_node ListNode;

typedef struct
{
	List *list;
	ListNode *node;
} ListIt;

int list_create(List **, size_t size);

void list_destroy(List *);

void list_clear(List *);

size_t list_sizeof(const List *);

bool list_empty(const List *);

size_t list_size(const List *);

int list_push_front(List *, const void *);

int list_push_back(List *, const void *);

int list_front(const List *, void *);

void *list_front_data(const List *);

int list_back(const List *, void *);

void *list_back_data(const List *);

int list_pop_front(List *);

int list_pop_back(List *);

void list_reverse(List *);

ListIt list_it_begin(const List *);

ListIt list_it_end(const List *);

bool list_it_equal(const ListIt *, const ListIt *);

bool list_it_inequal(const ListIt *, const ListIt *);

int list_it_insert(const ListIt *, const void *);

void list_it_erase(const ListIt *);

void list_it_get(const ListIt *, void *);

void list_it_set(const ListIt *, const void *);

void *list_it_data(const ListIt *);

void list_it_prev(ListIt *);

void list_it_next(ListIt *);

#endif