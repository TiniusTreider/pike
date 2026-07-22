#include "error.h"
#include "memory.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

void *smalloc(size_t size)
{
        errorif(size == 0, "Tried to allocate 0");

        void *pointer = malloc(size);
        errorif(pointer == NULL, "Out of memory");

        return pointer;
}

void *scalloc(size_t count, size_t size)
{
        errorif(size == 0, "0 in calloc size");
        errorif(count > SIZE_MAX / size, "Overflow in calloc size");

        void *pointer = calloc(count, size);
        errorif(pointer == NULL, "Out of memory");

        return pointer;
}

void *srealloc(void *data, size_t size)
{;
        if (size == 0) {
                free(data);
                return NULL;
        }

        void *pointer = realloc(data, size);
        errorif(pointer == NULL, "Out of memory");

        return pointer;
}

