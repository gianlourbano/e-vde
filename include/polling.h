#ifndef POLLING_H
#define POLLING_H

#include <pthread.h>
#include <sys/epoll.h>

typedef struct epoll_fds
{
    int epoll_fd;
    struct epoll_event *events;
    size_t nevents;
    size_t capacity;

    pthread_mutex_t mutex;

    const char *desc;
} epoll_fds;

#define EPOLL_DATA_INITIALIZER(name) {-1, NULL, 0, 0, PTHREAD_MUTEX_INITIALIZER, name}

void init_epoll(epoll_fds *efd, size_t capacity);

void add_fd(epoll_fds *efd, int fd, int type, void* data);
void remove_fd(epoll_fds *efd, int fd);

#endif