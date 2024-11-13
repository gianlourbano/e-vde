#ifndef MEMORY_H
#define MEMORY_H

typedef unsigned long size_t;

// arena allocator
typedef struct {
    void *ptr;
    size_t size;
    size_t cur;
} arena;

arena *arena_alloc(size_t size);
void arena_free(arena *ptr);
void *amalloc(arena* arena, size_t size);
void afree(void *ptr);


#endif