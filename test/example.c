#include <stdio.h>
#include "../src/fmemt.h"

struct test_struct {
	int x;
	int y;
	char string[256];
};

int main()
{
	/* initializing fmemt */
	fmemt_init();

	/* allocate memory with wrapper functions */
	void *ptr = mt_malloc(1000);
	struct test_struct *st = mt_malloc(sizeof(struct test_struct));
	char *array = mt_calloc(10, sizeof(char));

	mt_free(ptr);
	array = mt_reallocarray(array, 20, sizeof(char));

	/* destroy fmemt and report usage statistics */
	fmemt_destroy();

	return 0;
}
