#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "logging.h"

// arena allocator

arena* arena_alloc(size_t size) {
    arena* a = malloc(sizeof(arena));
    a->ptr = calloc(size, 1);
    a->size = size;
    a->cur = 0;
    return a;
}

void arena_free(arena *ptr) {
    free(ptr->ptr);
    free(ptr);
}

void *amalloc(arena* arena, size_t size) {
    if (arena->cur + size > arena->size) {
        printlog(ERROR, "Arena out of memory\n");
        exit(1);
    }
    void *ptr = arena->ptr + arena->cur;
    arena->cur += size;
    return ptr;
}

void afree(void *ptr) {
    // do nothing
}

