#ifndef RING_H
#define RING_H

#include <stdatomic.h>

#define RING_BUFFER_SIZE 1024  // Must be power of 2
#define BUFFER_MASK (RING_BUFFER_SIZE - 1)

typedef struct {
    void* data[RING_BUFFER_SIZE];
    atomic_size_t head;  // Write index
    atomic_size_t tail;  // Read index
} ring_buffer_t;

ring_buffer_t* ring_buffer_create(void);
void ring_buffer_free(ring_buffer_t* rb);
int ring_buffer_push(ring_buffer_t* rb, void* item);
int ring_buffer_pop(ring_buffer_t* rb, void** item);
size_t ring_buffer_size(ring_buffer_t* rb);

#endif