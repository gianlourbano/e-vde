#include "toeplitz.h"
#include "memory.h"
#include "utils.h"
#include "switch.h"
#include "ring.h"
#include "thread_pool.h"
#include "packet.h"
#include "logging.h"
#include "port.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Here we implement the forwarding engine. There should be:
 * - One function to initialize the forwarding engine, obv
 * - One thread handles the switching of the packets
 * - Several threads (maybe adjustable based on traffic?) to handle the actual packet processing
 *
 * So we need a thread pool for the packet processing, and a thread for the switching. I'd say the
 * thread for switching can be the same as the data plane
 *
 * we will use Toeplitz hashing to know which thread to send the packet to
 * lock free ring buffer for packet queues
 *
 */

thread_pool_t *pool = NULL;
extern endpoint *endpoints;

int fw_engine_init(int num_threads)
{
    toeplitz_init();
    pool = thread_pool_create(num_threads);
    return 0;
}

int dispatch_packet(int fd)
{
    packet* p = malloc(sizeof(packet));

   // int len = recvfrom(fd, &p, sizeof(packet), MSG_PEEK, NULL, NULL);
    int len = recv(fd, p, sizeof(packet), 0);

    uint32_t hash = compute_packet_hash((uint8_t*)p, len);

    int thread_idx = hash % pool->num_threads;

    //printlog(INFO, "Dispatching packet to thread %d", thread_idx);

    thread_pool_dispatch_packet(pool, p, thread_idx, fd);

    return 0;
}

int flood_packet(packet* p, int origin_port) {
    endpoint* ep = endpoints;
    while (ep != NULL) {
        if (ep->port != origin_port) {
            send(ep->fd_data, p, sizeof(packet), 0);
        }
        ep = ep->next;
    }
    return 0;
}

// todo : optimize tf is this
int send_packet(packet* p, int port) {
    endpoint* ep = endpoints;
    while (ep != NULL) {
        if (ep->port == port) {
            while(send(ep->fd_data, p, sizeof(packet), 0) < 0) {
                printlog(ERROR, "send() failed: %s", strerror(errno));
            }
            free(p);
            break;
        }
        ep = ep->next;
    }
    return 0;
}