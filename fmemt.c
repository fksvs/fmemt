#include <stdio.h>
#include <stdlib.h>
#include "fmemt.h"

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
		if (node->block->flag == MEM_FLAG_USE) {
			free(node->block->ptr);
		}
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
	fprintf(stdout, "total allocations : %ld\n", usage.total_allocs);
	fprintf(stdout, "total allocated memory : %ld\n", usage.total_alloc_mem);
	fprintf(stdout, "maximum allocated memory : %ld\n", usage.max_alloc_mem);
	fprintf(stdout, "minimum allocated memory : %ld\n", usage.min_alloc_mem);
	fprintf(stdout, "total frees : %ld\n", usage.total_frees);
	fprintf(stdout, "total freed memory : %ld\n", usage.total_freed_mem);
}

void report_leak_stat()
{
	struct node_t *node = memory_list->head;
	size_t total_leaks = 0;
	size_t total_leaked_mem = 0;

	while (node != NULL) {
		if (node->block->flag == MEM_FLAG_USE) {
			total_leaks++;
			total_leaked_mem += node->block->block_size;
			fprintf(stdout, "at %p, %ld bytes of memory leaked\n", 
				node->block->ptr, node->block->block_size);
		}

		node = node->next;
	}

	fprintf(stdout, "total leaks : %ld\n", total_leaks);
	fprintf(stdout, "total leaked memory : %ld\n", total_leaked_mem);
}

int fmemt_init()
{
	if (list_init() == -1) {
		return -1;
	}

	usage.total_allocs = 0;
	usage.total_alloc_mem = 0;
	usage.max_alloc_mem = 0;
	usage.min_alloc_mem = 0;
	usage.total_frees = 0;
	usage.total_freed_mem = 0;
}

void fmemt_destroy()
{
	report_usage_stat();
	report_leak_stat();
	list_destroy();
}

void *fmemt_malloc(size_t size)
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

	list_insert(block);

	usage.total_allocs++;
	usage.total_alloc_mem += size;
	if (size > usage.max_alloc_mem) {
		usage.max_alloc_mem = size;
	}

	if (size < usage.min_alloc_mem) {
		usage.min_alloc_mem = size;
	}

	return ptr;
}

void fmemt_free(void *ptr)
{
	if (memory_list) {
		struct memory_block *block = list_search(ptr);

		if (block) {
			block->flag = MEM_FLAG_FREE;

			usage.total_frees++;
			usage.total_freed_mem += block->block_size;
		}
	}
}

void *fmemt_calloc(size_t nmemb, size_t size)
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

	list_insert(block);

	usage.total_allocs++;
	usage.total_alloc_mem += size * nmemb;
	if (size * nmemb > usage.max_alloc_mem) {
		usage.max_alloc_mem = size * nmemb;
	}
	if (size * nmemb < usage.min_alloc_mem) {
		usage.min_alloc_mem = size * nmemb;
	}

	return ptr;
}

/*
void *fmemt_realloc(void *ptr, size_t size);
void *fmemt_reallocarray(void *ptr, size_t nmemb, size_t size);
*/
