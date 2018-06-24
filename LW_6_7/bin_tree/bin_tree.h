#ifndef _BIN_TREE_H_
#define _BIN_TREE_H_

#define BIN_TREE_SUCCESS       (0)
#define BIN_TREE_NO_MEMORY     (1 << 0)
#define BIN_TREE_ALREADY_EXIST (1 << 1)
#define BIN_TREE_NOT_EXIST     (1 << 2)

typedef struct bin_tree BinTree;

int bin_tree_create(BinTree **, size_t, void (*)(void *), int (*)(const void *, const void *));

void bin_tree_destroy(BinTree *);

void bin_tree_clear(BinTree *);

size_t bin_tree_sizeof(const BinTree *);

bool bin_tree_empty(const BinTree *);

size_t bin_tree_size(const BinTree *);

int bin_tree_insert(BinTree *, const void *);

int bin_tree_find(const BinTree *, const void *, void *);

void *bin_tree_data(const BinTree *, const void *);

int bin_tree_delete(BinTree *, const void *);

#endif