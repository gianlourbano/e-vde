#include "polling.h"
#include "data_plane.h"
#include "logging.h"
#include "utils.h"
#include "message.h"
#include "packet.h"
#include "fw_engine.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <port.h>

extern int ctl_pipe[2];
extern endpoint *endpoints;

epoll_fds data_plane_fds = EPOLL_DATA_INITIALIZER("data_plane");
pthread_t data_plane_thread;

int worker_threads = 4;

int data_plane_loop();


int init_data_plane()
{

    init_epoll(&data_plane_fds, 1);

    // Read messages from the control plane
    add_fd(&data_plane_fds, ctl_pipe[0], CTL_MESSAGE, NULL);

    if (pthread_create(&data_plane_thread, NULL, (void *(*)(void *))data_plane_loop, NULL) != 0)
    {
        printlog(ERROR, "pthread_create() failed: %s", strerror(errno));
        return -1;
    }

    if(fw_engine_init(worker_threads) < 0)
    {
        printlog(ERROR, "fw_engine_init() failed");
        return -1;
    }

    return 0;
}

int handle_ctl_message(int fd, int type)
{

    // read message type from upper 16 bits of type

    uint64_t msg_header;
    int n = read(fd, &msg_header, sizeof(uint64_t));

    if (n < 0)
    {
        printlog(ERROR, "read() failed: %s", strerror(errno));
        return -1;
    }

    int msg_type = HI(msg_header);
    int len = LO(msg_header);

    printlog(DEBUG, "Received message type %d, len %d", msg_type, len);

    if (n < 0)
    {
        printlog(ERROR, "read() failed: %s", strerror(errno));
        return -1;
    }

    switch (msg_type)
    {
    case MSG_PORT_UP:
        printlog(INFO, "Port up message received");
        break;
    case MSG_PORT_DOWN:
        printlog(INFO, "Port down message received");
        break;
    case MSG_PORT_ADD:

        int data_fd = -1;
        int n = read(fd, &data_fd, len);

        if (data_fd < 0)
        {
            printlog(ERROR, "read() failed: %s", strerror(errno));
            return -1;
        }

        add_fd(&data_plane_fds, data_fd, PACKET, NULL);

        printlog(INFO, "Port add message received");
        break;
    case MSG_PORT_DEL:
        printlog(INFO, "Port delete message received");
        break;
    default:
        printlog(ERROR, "Unknown message type %d", msg_type);
        break;
    }

    return 0;
}

int data_plane_loop()
{

    epoll_fds *efd = &data_plane_fds;

    int n;

    while (1)
    {

        n = epoll_wait(efd->epoll_fd, efd->events, efd->nevents, -1);

        printlog(DEBUG, "epoll_wait() returned %d events", n);

        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            printlog(ERROR, "epoll_wait() failed: %s", strerror(errno));
            return -1;
        }

        for (int i = 0; i < n; i++)
        {
            int fd = LO(efd->events[0].data.u64);
            int type = HI(efd->events[0].data.u64);

            switch (type)
            {
            case CTL_MESSAGE:
                handle_ctl_message(fd, type);
                break;
            case PACKET:
                dispatch_packet(fd);
                break;
            default:
                printlog(ERROR, "[DATA] Unknown event type %d", type);
                break;
            }
        }
    }
}

void wait_for_data()
{
    pthread_join(data_plane_thread, NULL);
}