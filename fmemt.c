#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fmemt.h"

#define MEM_FLAG_USE 0
#define MEM_FLAG_FREE 1

#define FREE 0
#define MALLOC 1
#define CALLOC 2
#define REALLOC 3
#define REALLOCARRAY 4

#define MAX_FILENAME 256

static const char *func_string[] = { "free()", "malloc()", "calloc()", "realloc()",
					"reallocarray()" };

struct memory_block {
        void *ptr;
        size_t block_size;
        int flag;
        int function;
	int line;
	char filename[MAX_FILENAME];
        struct memory_block *prev;
        struct memory_block *next;
};

typedef struct {
        size_t total_allocs;
	size_t total_alloc_mem;
	size_t total_frees;
	size_t total_freed_mem;
	struct memory_block *head;
        struct memory_block *tail;
} memory_list;

static memory_list *memory = NULL;

void report_usage_stat()
{
	fprintf(stdout, "\nheap usage statistics:\n");
	fprintf(stdout, " %ld allocations, %ld bytes allocated\n",
			memory->total_allocs, memory->total_alloc_mem);
	fprintf(stdout, " %ld frees, %ld bytes freed\n",
			memory->total_frees, memory->total_freed_mem);
}

void report_leak_stat()
{
	struct memory_block *block = memory->head;
	size_t total_leaks = 0;
	size_t total_leaked_mem = 0;

	fprintf(stdout, "\nleak statistics:\n");
	while (block != NULL) {
		if (block->flag == MEM_FLAG_USE) {
			total_leaks++;
			total_leaked_mem += block->block_size;
			fprintf(stdout, "  %ld bytes leaked\n  at %p, %s (%s:%d)\n\n",
					block->block_size, block->ptr,
					func_string[block->function],
					block->filename, block->line);
		}

		block = block->next;
	}

	fprintf(stdout, " %ld leaks, %ld bytes leaked\n\n", total_leaks, total_leaked_mem);
}


int fmemt_init()
{
	memory = malloc(sizeof(memory_list));
	if (!memory) return -1;

	memory->total_allocs = 0;
	memory->total_alloc_mem = 0;
	memory->total_frees = 0;
	memory->total_freed_mem = 0;
	memory->head = NULL;
	memory->tail = NULL;

	return 0;
}

void fmemt_destroy()
{
	struct memory_block *block = memory->head;

	report_usage_stat();
	report_leak_stat();

	while (block != NULL) {
		struct memory_block *temp = block->next;
		free(block);
		block = temp;
	}

	free(memory);
}

static struct memory_block *list_search(void *ptr)
{
	struct memory_block *block = memory->head;

	while (block != NULL) {
		if (block->ptr == ptr) return block;
		block = block->next;
	}

	return NULL;
}

static void list_update(void *ptr, size_t size, int flag,
			int func, int line, const char *file)
{
	struct memory_block *block;

	if ((block = list_search(ptr)) == NULL) {
		block = malloc(sizeof(struct memory_block));
		block->prev = NULL;
		block->next = NULL;

		if (memory->head == NULL || memory->tail == NULL) {
			memory->head = block;
			memory->tail = block;
		} else {
			block->prev = memory->tail;
			memory->tail->next = block;
			memory->tail = block;
		}
	}

	block->ptr = ptr;
	block->block_size = size;
	block->flag = flag;
	block->function = func;
	block->line = line;
	strncpy(block->filename, file, strlen(file));
}

void update_usage(int mode, size_t size)
{
	if (mode != FREE) {
		memory->total_allocs++;
		memory->total_alloc_mem += size;
	} else {
		memory->total_frees++;
		memory->total_freed_mem += size;
	}
}

void *fmemt_malloc(size_t size, const char *filename, int line)
{
	void *ptr;

	if (!memory) fmemt_init();

	if ((ptr = malloc(size)) == NULL) return ptr;

	list_update(ptr, size, MEM_FLAG_USE, MALLOC, line, filename);
	update_usage(MALLOC, size);

	return ptr;
}

void fmemt_free(void *ptr, const char *filename, int line)
{
	if (memory) {
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
	void *ptr;

	if (!memory) fmemt_init();

	if ((ptr = calloc(nmemb, size)) == NULL) return ptr;

	list_update(ptr, nmemb * size, MEM_FLAG_USE, CALLOC, line, filename);
	update_usage(CALLOC, nmemb  * size);

	return ptr;
}

void *fmemt_realloc(void *ptr, size_t size, const char *filename, int line)
{
	struct memory_block *block;
	void *new_ptr;
	size_t size_diff = size;

	if (!memory) fmemt_init();

	if ((new_ptr = realloc(ptr, size)) == NULL) return new_ptr;

	if ((block = list_search(ptr)) != NULL) {
		size_diff = size > block->block_size ? size - block->block_size : 0;
	}

	list_update(new_ptr, size, MEM_FLAG_USE, REALLOC, line, filename);
	update_usage(REALLOC, size_diff);

	return new_ptr;
}

void *fmemt_reallocarray(void *ptr, size_t nmemb, size_t size,
			const char *filename, int line)
{
	struct memory_block *block;
	void *new_ptr;
	size_t size_diff = size;

	if (!memory) fmemt_init();

	if ((new_ptr = reallocarray(ptr, nmemb, size)) == NULL) return new_ptr;

	if ((block = list_search(ptr)) != NULL ) {
		size_diff = size * nmemb > block->block_size ? size * nmemb -
				block->block_size : 0;
	}

	list_update(new_ptr, nmemb * size, MEM_FLAG_USE, REALLOCARRAY, line, filename);
	update_usage(REALLOCARRAY, size_diff);

	return new_ptr;
}
