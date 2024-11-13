#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdint.h>

#define MAX_THREADS 4
#define PIPE_READ 0
#define PIPE_WRITE 1

// Message types for pipe communication
typedef enum {
    MSG_PROCESS_PACKET,
    MSG_SHUTDOWN
} message_type_t;

typedef struct {
    uint64_t type;
    void* data;
} thread_message_t;

typedef struct {
    pthread_t thread;
    int pipe_fd[2];
    ring_buffer_t* ring;
    int running;

    int thread_idx;
} worker_t;

typedef struct {
    worker_t workers[MAX_THREADS];
    size_t num_threads;
} thread_pool_t;


thread_pool_t* thread_pool_create(size_t num_threads);
void thread_pool_dispatch_packet(thread_pool_t* pool, void* packet, size_t thread_idx, int fd);
void thread_pool_destroy(thread_pool_t* pool);

#endif