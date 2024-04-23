#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fmemt.h"

#define MEM_FLAG_USE 0
#define MEM_FLAG_FREE 1

enum {
	FREE,
	MALLOC,
	CALLOC,
	REALLOC,
	REALLOCARRAY
};

static const char *func_string[] = { "free()", "malloc()", "calloc()", "realloc()",
					"reallocarray()" };

#define MAX_FILENAME 256

struct memory_block {
        void *ptr;
        size_t block_size;
        int flag;
        int function;
	int line;
	char filename[MAX_FILENAME];
};

struct node_t {
        struct memory_block *block;
        struct node_t *prev;
        struct node_t *next;
};

typedef struct {
        struct node_t *head;
        struct node_t *tail;
} list_t;

struct usage_stat {
        size_t total_allocs;
        size_t total_alloc_mem;
        size_t total_frees;
        size_t total_freed_mem;
};

static list_t *memory_list = NULL;
static struct usage_stat usage;

static int list_init()
{
	memory_list = malloc(sizeof(list_t));
	if (!memory_list) {
		return -1;
	}

	memory_list->head = NULL;
	memory_list->tail = NULL;

	return 0;
}

static void list_destroy()
{
	struct node_t *node = memory_list->head;

	while (node != NULL) {
		struct node_t *temp = node->next;
		free(node->block);
		free(node);
		node = temp;
	}

	free(memory_list);
}

static struct node_t *init_list_data(struct memory_block *block)
{
	struct node_t *node = malloc(sizeof(struct node_t));

	node->block = block;
	node->prev = NULL;
	node->next = NULL;

	return node;
}

static struct node_t *list_insert(struct memory_block *block)
{
	struct node_t *node = init_list_data(block);

	if (memory_list->head == NULL || memory_list->tail == NULL) {
		memory_list->head = node;
		memory_list->tail = node;
	} else {
		node->prev = memory_list->tail;
		memory_list->tail->next = node;
		memory_list->tail = node;
	}

	return node;
}

static struct memory_block *list_search(void *ptr)
{
	struct node_t *node = memory_list->head;

	while (node != NULL) {
		if (node->block->ptr == ptr) {
			return node->block;
		}
		node = node->next;
	}

	return NULL;
}

void report_usage_stat()
{
	fprintf(stdout, "\nheap usage statistics:\n");
	fprintf(stdout, " %ld allocations, %ld bytes allocated\n",
			usage.total_allocs, usage.total_alloc_mem);
	fprintf(stdout, " %ld frees, %ld bytes freed\n", 
			usage.total_frees, usage.total_freed_mem);
}

void report_leak_stat()
{
	struct node_t *node = memory_list->head;
	size_t total_leaks = 0;
	size_t total_leaked_mem = 0;

	fprintf(stdout, "\nleak statistics:\n");
	while (node != NULL) {
		if (node->block->flag == MEM_FLAG_USE) {
			total_leaks++;
			total_leaked_mem += node->block->block_size;
			fprintf(stdout, "  %ld bytes leaked\n  at %p, %s (%s:%d)\n\n",
					node->block->block_size, node->block->ptr,
					func_string[node->block->function],
					node->block->filename, node->block->line);
		}

		node = node->next;
	}

	fprintf(stdout, " %ld leaks, %ld bytes leaked\n\n", total_leaks, total_leaked_mem);
}

int fmemt_init()
{
	if (list_init() == -1) {
		return -1;
	}

	usage.total_allocs = 0;
	usage.total_alloc_mem = 0;
	usage.total_frees = 0;
	usage.total_freed_mem = 0;
}

void fmemt_destroy()
{
	report_usage_stat();
	report_leak_stat();
	list_destroy();
}

void update_usage(int mode, size_t size)
{
	if (mode != FREE) {
		usage.total_allocs++;
		usage.total_alloc_mem += size;
	} else {
		usage.total_frees++;
		usage.total_freed_mem += size;
	}
}

void *fmemt_malloc(size_t size, const char *filename, int line)
{
	struct memory_block *block = malloc(sizeof(struct memory_block));
	void *ptr;

	if (!memory_list) {
		list_init();
	}

	if ((ptr = malloc(size)) == NULL) {
		return ptr;
	}

	block->ptr = ptr;
	block->block_size = size;
	block->flag = MEM_FLAG_USE;
	block->function = MALLOC;
	block->line = line;
	strncpy(block->filename, filename, strlen(filename));

	list_insert(block);
	update_usage(MALLOC, size);

	return ptr;
}

void fmemt_free(void *ptr, const char *filename, int line)
{
	if (memory_list) {
		struct memory_block *block = list_search(ptr);

		if (block) {
			block->flag = MEM_FLAG_FREE;
			block->function = FREE;
			strncpy(block->filename, filename, strlen(filename));
			block->line = line;

			update_usage(FREE, block->block_size);
		}
	}
}

void *fmemt_calloc(size_t nmemb, size_t size, const char *filename, int line)
{
	struct memory_block *block = malloc(sizeof(struct memory_block));
	void *ptr;

	if (!memory_list) {
		list_init();
	}

	if ((ptr = calloc(nmemb, size)) == NULL) {
		return ptr;
	}

	block->ptr = ptr;
	block->block_size = size;
	block->flag = MEM_FLAG_USE;
	block->function = CALLOC;
	block->line = line;
	strncpy(block->filename, filename, strlen(filename));

	list_insert(block);
	update_usage(CALLOC, size);

	return ptr;
}

void *fmemt_realloc(void *ptr, size_t size, const char *filename, int line)
{
	struct memory_block *block;
	void *new_ptr;

	if (!memory_list) {
		list_init();
	}

	if ((new_ptr = realloc(ptr, size)) == NULL) {
		return new_ptr;
	}

	block = list_search(ptr);
	if (block) {
		block->ptr = new_ptr;
		block->block_size = size;
		block->flag = MEM_FLAG_USE;
		block->function = REALLOC;
		block->line = line;
		strncpy(block->filename, filename, strlen(filename));
	} else if (!block){
		block = malloc(sizeof(struct memory_block));

		block->ptr = new_ptr;
		block->block_size = size;
		block->flag = MEM_FLAG_USE;
		block->function = REALLOC;
		block->line = line;
		strncpy(block->filename, filename, strlen(filename));

		list_insert(block);
	}
	update_usage(REALLOC, size);

	return new_ptr;
}

void *fmemt_reallocarray(void *ptr, size_t nmemb, size_t size,
			const char *filename, int line)
{
	struct memory_block *block;
	void *new_ptr;

	if (!memory_list) {
		list_init();
	}

	if ((new_ptr = reallocarray(ptr, nmemb, size)) == NULL) {
		return new_ptr;
	}

	block = list_search(ptr);
	if (block) {
		block->ptr = new_ptr;
		block->block_size = size;
		block->flag = MEM_FLAG_USE;
		block->function = REALLOCARRAY;
		block->line = line;
		strncpy(block->filename, filename, strlen(filename));
	} else if (!block) {
		block = malloc(sizeof(struct memory_block));

		block->ptr = new_ptr;
		block->block_size = nmemb * size;
		block->flag = MEM_FLAG_USE;
		block->function = REALLOCARRAY;
		block->line = line;
		strncpy(block->filename, filename, strlen(filename));

		list_insert(block);
	}
	update_usage(REALLOCARRAY, nmemb * size);

	return new_ptr;
}
