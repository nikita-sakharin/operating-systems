#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

typedef struct queue_node QueueNode;

struct queue_node
{
	QueueNode *next;
	uint_least8_t data[];
};

struct queue
{
	QueueNode *head;
	QueueNode *tail;
	size_t size;
	size_t count;
};

#define _QUEUE_H_SAVE_

int queue_create(register Queue ** restrict queue_ptr, size_t size)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue_ptr == NULL || !size)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register Queue *temp = aligned_alloc(alignof(Queue), sizeof(Queue));
	if (temp == NULL)
	{
		return QUEUE_NOMEM;
	}

	temp->head = temp->tail = NULL;
	temp->size = size;
	temp->count = 0u;

	*queue_ptr = temp;

	return QUEUE_SUCCESS;
}

void queue_destroy(register Queue * restrict queue)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	for (register QueueNode *temp = queue->head, *local_temp; temp != NULL; temp = local_temp)
	{
		local_temp = temp->next;
		free(temp);
	}

	free(queue);
}

void queue_clear(register Queue * restrict queue)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	for (register QueueNode *temp = queue->head, *local_temp; temp != NULL; temp = local_temp)
	{
		local_temp = temp->next;
		free(temp);
	}

	queue->head = queue->tail = NULL;
	queue->count = 0u;
}

size_t queue_sizeof(register const Queue * restrict queue)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return queue->size;
}

bool queue_empty(register const Queue * restrict queue)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return !queue->count;
}

size_t queue_size(register const Queue * restrict queue)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	return queue->count;
}

int queue_push(register Queue * restrict queue, const void * restrict source)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL || source == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	register QueueNode *temp = aligned_alloc(alignof(QueueNode), sizeof(QueueNode) + queue->size);
	if (temp == NULL)
	{
		return QUEUE_NOMEM;
	}

	temp->next = NULL;
	memcpy(temp->data, source, queue->size);

	if (queue->head == NULL)
	{
		queue->head = temp;
	}
	else
	{
		queue->tail->next = temp;
	}
	queue->tail = temp;
	++queue->count;

	return QUEUE_SUCCESS;
}

int queue_front(register const Queue * restrict queue, void * restrict dest)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL || dest == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (queue->head == NULL)
	{
		return QUEUE_EMPTY;
	}

	memcpy(dest, queue->head->data, queue->size);

	return QUEUE_SUCCESS;
}

int queue_back(register const Queue * restrict queue, void * restrict dest)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL || dest == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (queue->tail == NULL)
	{
		return QUEUE_EMPTY;
	}

	memcpy(dest, queue->tail->data, queue->size);

	return QUEUE_SUCCESS;
}

void *queue_front_data(register const Queue * restrict queue)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (queue->head == NULL)
	{
		return NULL;
	}

	return queue->head->data;
}

void *queue_back_data(register const Queue * restrict queue)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (queue->tail == NULL)
	{
		return NULL;
	}

	return queue->tail->data;
}

int queue_pop(register Queue * restrict queue)
{
#	ifdef _QUEUE_H_SAVE_
	if (queue == NULL)
	{
		exit(EXIT_FAILURE);
	}
#	endif

	if (queue->head == NULL)
	{
		return QUEUE_EMPTY;
	}

	register QueueNode *temp = queue->head;

	queue->head = temp->next;
	if (temp->next == NULL)
	{
		queue->tail = NULL;
	}
	--queue->count;

	free(temp);

	return QUEUE_SUCCESS;
}