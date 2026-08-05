#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void *smalloc(size_t size);
void *scalloc(size_t count, size_t size);
void *srealloc(void *data, size_t size);

#endif

