#include "polling.h"
#include "logging.h"
#include "utils.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

void init_epoll(epoll_fds *efd, size_t capacity)
{
    efd->epoll_fd = epoll_create1(0);
    efd->events = malloc(sizeof(struct epoll_event) * capacity);
    efd->nevents = 0;
    efd->capacity = capacity;
    efd->desc = "epoll";

    pthread_mutex_init(&efd->mutex, NULL);
}

void add_fd(epoll_fds *efd, int fd, int type, void *data)
{
    printlog(DEBUG, "Adding fd %d to epoll %s", fd, efd->desc);

    pthread_mutex_lock(&efd->mutex);

    struct epoll_event event;
    event.events = EPOLLIN;

    if (data != NULL)
    {
        event.data.ptr = data; // for data plane
    }
    else
    {
        event.data.u64 = TO64(type, fd);
    }

    if (epoll_ctl(efd->epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        pthread_mutex_unlock(&efd->mutex);
        printlog(ERROR, "Failed to add fd %d to epoll %s: %s", fd, efd->desc, strerror(errno));
        return;
    }

    efd->nevents++;

    pthread_mutex_unlock(&efd->mutex);
}

void remove_fd(epoll_fds *efd, int fd)
{
    printlog(DEBUG, "Removing fd %d from epoll %s", fd, efd->desc);

    pthread_mutex_lock(&efd->mutex);

    if (epoll_ctl(efd->epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        pthread_mutex_unlock(&efd->mutex);
        printlog(ERROR, "Failed to remove fd %d from epoll %s: %s", fd, efd->desc, strerror(errno));
        return;
    }

    efd->nevents--;

    pthread_mutex_unlock(&efd->mutex);
}