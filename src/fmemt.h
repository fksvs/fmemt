#ifndef FMEMT_H
#define FMEMT_H

#include <sys/types.h>

void report_usage_stat();
void report_leak_stat();

int fmemt_init();
void fmemt_destroy();

void *fmemt_malloc(size_t size, const char *filenanme, int line);
void fmemt_free(void *ptr, const char *filename, int line);
void *fmemt_calloc(size_t nmemb, size_t size, const char *filename, int line);
void *fmemt_realloc(void *ptr, size_t size, const char *filename, int line);
void *fmemt_reallocarray(void *ptr, size_t nmemb, size_t size,
			const char *filename, int line);

#define mt_malloc(SIZE) fmemt_malloc(SIZE, __FILE__, __LINE__)
#define mt_free(PTR) fmemt_free(PTR, __FILE__, __LINE__)
#define mt_calloc(NMEMB, SIZE) fmemt_calloc(NMEMB, SIZE, __FILE__, __LINE__)
#define mt_realloc(PTR, SIZE) fmemt_realloc(PTR, SIZE, __FILE__, __LINE__)
#define mt_reallocarray(PTR, NMEMB, SIZE) fmemt_reallocarray(PTR, NMEMB, SIZE, __FILE__, __LINE__)

#endif
