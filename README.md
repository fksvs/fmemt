FMEMT - Dynamic Memory Tracker
==============================

fmemt is a dynamic memory tracker designed to monitor heap usage and detect memory leaks. It is built on top of standard memory allocation functions (malloc, calloc, realloc, reallocarray and free). 

## Table of Contents

1. [Installation](#installation)
2. [Usage](#usage)
3. [Example](#example)
4. [Caveats](#caveats)
5. [License](#license)

## Installation

- Clone the repository from [GitHub][] or [GitLab][]:

```console
git clone https://github.com/fksvs/fmemt
git clone https://gitlab.com/fksvs/fmemt
```

- Change directory to `fmemt`:

```console
cd fmemt/
```

- Copy `fmemt.c` and `fmemt.h` from `src/` to your project directory:

```console
cp src/fmemt.* [your project directory]
```

- Include `fmemt.h` in your project and compile with it.

## Usage

Utilize the fmemt wrapper functions within your project. Upon program completion, detailed statistics on heap memory usage and leaks will be reported.

Initialization and destruction functions:
```c
int fmemt_init();
void fmemt_destroy();
```

Wrapper functions:

```c
void *fmemt_malloc(size_t size, const char *filenanme, int line);
void fmemt_free(void *ptr, const char *filename, int line);
void *fmemt_calloc(size_t nmemb, size_t size, const char *filename, int line);
void *fmemt_realloc(void *ptr, size_t size, const char *filename, int line);
void *fmemt_reallocarray(void *ptr, size_t nmemb, size_t size, const char *filename, int line);
```
Macros defined for simplified use:

```c
mt_malloc(SIZE) fmemt_malloc(SIZE, __FILE__, __LINE__)
mt_free(PTR) fmemt_free(PTR, __FILE__, __LINE__);
mt_calloc(NMEMB, SIZE) fmemt_calloc(NMEMB, SIZE, __FILE__, __LINE__)
mt_realloc(PTR, SIZE) fmemt_realloc(PTR, SIZE, __FILE__, __LINE__)
mt_reallocarray(PTR, NMEMB, SIZE) fmemt_reallocarray(PTR, NMEMB, SIZE, __FILE__, __LINE__)
```
Heap usage and leak statistics can be dumped using:

```c
void report_usage_stat();
void report_leak_stat();
```

## Example

Sample program located in `test/` directroy. Build it with:

```console
make
```
Change directory to `test/` and run the example:

```console
cd test/
./example
```

Example C program: 

```c
#include <stdio.h>
#include "fmemt.h"

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
```

## Caveats

fmemt is unable to track memory allocated directly with malloc, calloc, etc. It is essential to use fmemt wrapper functions to monitor all allocations.

## License

This project is free software; you can redistribute it and/or modify it under the terms of the GPLv3 license. See [LICENSE][] for details.

[GitHub]: https://github.com/fksvs/fmemt
[GitLab]: https://gitlab.com/fksvs/fmemt
[LICENSE]: https://www.gnu.org/licenses/gpl-3.0.en.html 
