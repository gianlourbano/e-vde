#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <errno.h>

#include "ring.h"
#include "logging.h"
#include "thread_pool.h"
#include "packet.h"
#include "hash_table.h"
#include "fw_engine.h"
#include "utils.h"
#include "port.h"
#include "hash_table.h"

extern fd_table_t *fdt;
extern mac_table_t *mt;

size_t n_worker_threads = 1;

// Worker thread function
static void *worker_thread(void *arg)
{
    worker_t *worker = (worker_t *)arg;
    thread_message_t msg;

    while (worker->running)
    {
        // Wait for message from dispatcher
        ssize_t bytes = read(worker->pipe_fd[PIPE_READ], &msg, sizeof(msg));
        if (bytes != sizeof(msg))
        {
            printlog(ERROR, "Failed to read message from pipe");
            continue;
        }

        message_type_t type = HI(msg.type);
        int fd = LO(msg.type);

        switch (type)
        {
        case MSG_PROCESS_PACKET:
            // Process packet from ring buffer

            //printlog(INFO, "Ring buffer size thread %d: %d", worker->thread_idx, ring_buffer_size(worker->ring));

            void *p;
            while (ring_buffer_pop(worker->ring, &p))
            {
                //printlog(INFO, "Processing packet on thread %d", worker->thread_idx);

                endpoint *ep = fd_table_lookup(fdt, fd);

                if (ep == NULL)
                {
                    printlog(ERROR, "Endpoint not found for fd %d", fd);
                    continue;
                }

                // print packet src and dest

                if (((packet *)p)->eth.proto[0] == 0x86 && ((packet *)p)->eth.proto[1] == 0xdd)
                {
                   // printlog(INFO, "IPv6 packet");
                    continue;
                }

                if (IS_BROADCAST(((packet *)p)->eth.dest))
                {
                    flood_packet((packet *)p, ep->port);
                }

                else
                {
                    //printlog(INFO, "Packet src: %02x:%02x:%02x:%02x:%02x:%02x", ((packet *)p)->eth.src[0], ((packet *)p)->eth.src[1], ((packet *)p)->eth.src[2], ((packet *)p)->eth.src[3], ((packet *)p)->eth.src[4], ((packet *)p)->eth.src[5]);
                    //printlog(INFO, "Packet dest: %02x:%02x:%02x:%02x:%02x:%02x", ((packet *)p)->eth.dest[0], ((packet *)p)->eth.dest[1], ((packet *)p)->eth.dest[2], ((packet *)p)->eth.dest[3], ((packet *)p)->eth.dest[4], ((packet *)p)->eth.dest[5]);
                    //printlog(INFO, "Protocol: %02x%02x", ((packet *)p)->eth.proto[0], ((packet *)p)->eth.proto[1]);

                    // deep packet inspection
                    //deep_packet_inspect((packet *)p);

                    int out_port = mac_table_lookup(mt, ((packet *)p)->eth.dest, 1);
                    if (ep->port == out_port)
                    {
                        continue;
                    }

                    int in_port = mac_table_lookup(mt, ((packet *)p)->eth.src, 1);
                    if (in_port == -1)
                    {
                        mac_table_insert(mt, ((packet *)p)->eth.src, 1, ep->port);
                    }

                    if (out_port == -1)
                    {
                        flood_packet((packet *)p, ep->port);
                    }
                    else
                    {
                      //  printlog(INFO, "Sending packet from port %d to port %d", ep->port, out_port);
                        send_packet((packet *)p, out_port);

                        // port *p = get_port(out_port);

                        // printlog(INFO, "sending packet on fd: %d", p->ep->fd_data);
                        // while(send(p->ep->fd_data, p, sizeof(packet), 0) < 0) {
                        //     printlog(ERROR, "send() failed: %s", strerror(errno));
                        // }
                        // free((packet*)p);
                    }
                }
               // printlog(WARNING, "Done processing packets on thread %d, buffer size: %d", worker->thread_idx, ring_buffer_size(worker->ring));
            }

            break;

        case MSG_SHUTDOWN:
            worker->running = false;
            break;
        }
    }
    return NULL;
}

thread_pool_t *thread_pool_create(size_t num_threads)
{
    if ((n_worker_threads = num_threads) < 1)
    {
        printlog(ERROR, "Can't have less than one worker thread");
        return NULL;
    }

    thread_pool_t *pool = calloc(1, sizeof(thread_pool_t));
    pool->num_threads = num_threads;

    // Initialize workers
    for (size_t i = 0; i < num_threads; i++)
    {
        worker_t *worker = &pool->workers[i];

        // Create pipe
        if (pipe(worker->pipe_fd) == -1)
        {
            // Handle error
            continue;
        }

        // Create ring buffer
        worker->ring = ring_buffer_create();
        worker->running = true;

        worker->thread_idx = i;

        // Create worker thread
        pthread_create(&worker->thread, NULL, worker_thread, worker);
    }

    return pool;
}

void thread_pool_dispatch_packet(thread_pool_t *pool, void *packet, size_t thread_idx, int fd)
{
    worker_t *worker = &pool->workers[thread_idx];

    // Try to add packet to ring buffer until successful
    while (!ring_buffer_push(worker->ring, packet))
    {
        
    }

    // Notify worker
    thread_message_t msg = {
        .type = TO64(MSG_PROCESS_PACKET, fd),
        .data = NULL};
    write(worker->pipe_fd[PIPE_WRITE], &msg, sizeof(msg));
}

void thread_pool_destroy(thread_pool_t *pool)
{
    // Send shutdown message to all workers
    thread_message_t msg = {
        .type = MSG_SHUTDOWN,
        .data = NULL};

    for (size_t i = 0; i < pool->num_threads; i++)
    {
        worker_t *worker = &pool->workers[i];
        write(worker->pipe_fd[PIPE_WRITE], &msg, sizeof(msg));
        pthread_join(worker->thread, NULL);

        // Cleanup
        close(worker->pipe_fd[PIPE_READ]);
        close(worker->pipe_fd[PIPE_WRITE]);
        ring_buffer_free(worker->ring);
    }

    free(pool);
}