#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdbool.h>

#define QUEUE_SUCCESS 0
#define QUEUE_NOMEM 1
#define QUEUE_EMPTY 2

typedef struct queue Queue;

int queue_create(Queue **, size_t);

void queue_destroy(Queue *);

void queue_clear(Queue *);

size_t queue_sizeof(const Queue *);

bool queue_empty(const Queue *);

size_t queue_size(const Queue *);

int queue_push(Queue *, const void *);

int queue_front(const Queue *, void *);

int queue_back(const Queue *, void *);

void *queue_front_data(const Queue *);

void *queue_back_data(const Queue *);

int queue_pop(Queue *);

#endif