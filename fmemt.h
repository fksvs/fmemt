#ifndef FMEMT_H
#define FMEMT_H

#include <sys/types.h>

void report_usage_stat();
void report_leak_stat();

/* fmemt initializer and destroyer */
int fmemt_init();
void fmemt_destroy();

/* wrapper functions for dynamic memory functions */
void *fmemt_malloc(size_t size);
void fmemt_free(void *ptr);
void *fmemt_calloc(size_t nmemb, size_t size);
void *fmemt_realloc(void *ptr, size_t size);
void *fmemt_reallocarray(void *ptr, size_t nmemb, size_t size);

#endif
