#ifndef FMEMT_H
#define FMEMT_H

#include <sys/types.h>

#define MEM_FLAG_USE 0
#define MEM_FLAG_FREE 1

struct memory_block {
	void *ptr; /* memory block location */
	size_t block_size; /* memory block size*/
	int flag; /* block usage flag, 0 for in use, 1 for freed memory */
};

/* list node */
struct node_t {
	struct memory_block *block;
	struct node_t *prev;
	struct node_t *next;
};

/* linked list struct */
typedef struct {
	struct node_t *head;
	struct node_t *tail;
} list_t;

/* list iteration struct */
typedef struct {
	struct node_t *cur;
} iter_t;

struct usage_stat {
	size_t total_allocs; /* total memory allocations */
	size_t total_alloc_mem; /* total allocated memory */
	size_t max_alloc_mem; /* maximum allocated memory */
	size_t min_alloc_mem; /* minimum allocated memory */
	size_t total_frees; /* total memory frees */
	size_t total_freed_mem; /* total freed memory */
};

void report_usage_stat();
void report_leak_stat();

/* fmemt initializer and destroyer */
int fmemt_init();
void fmemt_destroy();

/* wrapper functions for dynamic memory functions */
void *fmemt_malloc(size_t size);
void fmemt_free(void *ptr);
void *fmemt_calloc(size_t nmemb, size_t size);

/*
void *fmemt_realloc(void *ptr, size_t size);
void *fmemt_reallocarray(void *ptr, size_t nmemb, size_t size);
*/

#endif
