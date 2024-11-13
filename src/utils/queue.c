#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ring.h"
#include "switch.h"

ring_buffer_t* ring_buffer_create(void) {
    ring_buffer_t* rb = calloc(1, sizeof(ring_buffer_t));
    atomic_store(&rb->head, 0);
    atomic_store(&rb->tail, 0);
    return rb;
}

void ring_buffer_free(ring_buffer_t* rb) {
    free(rb);
}

int ring_buffer_push(ring_buffer_t* rb, void* item) {
    const size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    const size_t next_head = (head + 1) & BUFFER_MASK;
    
    if (next_head == atomic_load_explicit(&rb->tail, memory_order_acquire)) {
        return false;  // Buffer full
    }

    rb->data[head] = item;
    atomic_store_explicit(&rb->head, next_head, memory_order_release);
    return true;
}

int ring_buffer_pop(ring_buffer_t* rb, void** item) {
    const size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    
    if (tail == atomic_load_explicit(&rb->head, memory_order_acquire)) {
        return false;  // Buffer empty
    }

    *item = rb->data[tail];
    atomic_store_explicit(&rb->tail, (tail + 1) & BUFFER_MASK, memory_order_release);
    return true;
}

size_t ring_buffer_size(ring_buffer_t* rb) {
    return (atomic_load_explicit(&rb->head, memory_order_relaxed) - 
            atomic_load_explicit(&rb->tail, memory_order_relaxed)) & BUFFER_MASK;
}