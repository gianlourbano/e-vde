#include "switch.h"
#include "logging.h"
#include "control_plane.h"
#include "polling.h"
#include "utils.h"
#include "message.h"
#include "port.h"
#include "hash_table.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/stat.h>

extern int ctl_pipe[2];

epoll_fds ctl_plane_fds = EPOLL_DATA_INITIALIZER("control_plane");

char ctl_socket[PATH_MAX];

mac_table_t *mt;

int init_ctl_plane()
{
    int mode = -1;
    int dirmode = -1;
    gid_t grp_owner = -1;

    const char *rel_ctl_sock_path = NULL;

    int connect_fd = -1;

    int one = 1;

    struct sockaddr_un sun;
    const size_t max_ctl_sock_len = sizeof(sun.sun_path) - 1;

    if (mode < 0 && dirmode < 0)
    {
        mode = 00660;
        dirmode = 02770;
    }

    if ((connect_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        printlog(ERROR, "socket() failed");
        return -1;
    }

    if ((setsockopt(connect_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) < 0)
    {
        printlog(ERROR, "setsockopt() failed");
        return -1;
    }

    if (fcntl(connect_fd, F_SETFD, FD_CLOEXEC) < 0)
    {
        printlog(ERROR, "fcntl() failed");
        return -1;
    }

    if (rel_ctl_sock_path == NULL)
    {
        rel_ctl_sock_path = (geteuid() == 0) ? VDESTDSOCK : VDETMPSOCK;
    }

    if (((mkdir(rel_ctl_sock_path, 0777) < 0) && (errno != EEXIST)))
    {
        fprintf(stderr, "Cannot create ctl directory '%s': %s\n",
                rel_ctl_sock_path, strerror(errno));
        exit(-1);
    }
    if (!vde_realpath(rel_ctl_sock_path, ctl_socket))
    {
        fprintf(stderr, "Cannot resolve ctl dir path '%s': %s\n",
                rel_ctl_sock_path, strerror(errno));
        exit(1);
    }

    if (chown(ctl_socket, -1, grp_owner) < 0)
    {
        rmdir(ctl_socket);
        printlog(ERROR, "Could not chown socket '%s': %s", sun.sun_path, strerror(errno));
        exit(-1);
    }
    if (chmod(ctl_socket, dirmode) < 0)
    {
        printlog(ERROR, "Could not set the VDE ctl directory '%s' permissions: %s", ctl_socket, strerror(errno));
        exit(-1);
    }
    sun.sun_family = AF_UNIX;
    if (strlen(ctl_socket) > max_ctl_sock_len)
        ctl_socket[max_ctl_sock_len] = 0;
    snprintf(sun.sun_path, sizeof(sun.sun_path), "%s/ctl", ctl_socket);
    if (bind(connect_fd, (struct sockaddr *)&sun, sizeof(sun)) < 0)
    {
        if ((errno == EADDRINUSE) && still_used(&sun))
        {
            printlog(ERROR, "Could not bind to socket '%s/ctl': %s", ctl_socket, strerror(errno));
            exit(-1);
        }
        else if (bind(connect_fd, (struct sockaddr *)&sun, sizeof(sun)) < 0)
        {
            printlog(ERROR, "Could not bind to socket '%s/ctl' (second attempt): %s", ctl_socket, strerror(errno));
            exit(-1);
        }
    }
    chmod(sun.sun_path, mode);
    if (chown(sun.sun_path, -1, grp_owner) < 0)
    {
        printlog(ERROR, "Could not chown socket '%s': %s", sun.sun_path, strerror(errno));
        exit(-1);
    }
    if (listen(connect_fd, 15) < 0)
    {
        printlog(ERROR, "Could not listen on fd %d: %s", connect_fd, strerror(errno));
        exit(-1);
    }

    init_epoll(&ctl_plane_fds, 1);
    add_fd(&ctl_plane_fds, connect_fd, CTL_LISTEN, NULL);

    if (pipe(ctl_pipe) < 0)
    {
        printlog(ERROR, "pipe() failed: %s", strerror(errno));
        return -1;
    }

    mt = mac_table_create();

    return 0;
}

static int accept_new_ctl_conn(int fd)
{
    struct sockaddr addr;
    socklen_t len;
    len = sizeof(addr);
    printlog(INFO, "Received event on fd %d", fd);
    int client_fd = accept(fd, &addr, &len);
    printlog(INFO, "Accepted connection from client on fd %d", client_fd);
    if (client_fd < 0)
    {
        printlog(ERROR, "accept() failed");
        return -1;
    }
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        printlog(WARNING, "fcntl - setting O_NONBLOCK %s", strerror(errno));
        close(client_fd);
        return -1;
    }
    add_fd(&ctl_plane_fds, client_fd, CTL_WD, NULL);
    printlog(INFO, "Accepted connection from client");

    return 0;
}

static int handle_wd_req(int fd)
{
    char reqbuf[REQBUFLEN + 1];
    union request *req = (union request *)reqbuf;
    int len;

    len = read(fd, reqbuf, REQBUFLEN);
    if (len < 0)
    {
        printlog(ERROR, "read() failed");
        return -1;
    }

    else if (len == 0)
    {
        printlog(INFO, "Client closed connection");
        // remove_fd(info->fd);
        // epoll_ctl(ep->epoll_fd, EPOLL_CTL_DEL, info->fd, NULL);
        close(fd);
        return 1;
    }

    else if (req->v1.magic == SWITCH_MAGIC)
    {
        struct sockaddr_un sa_un;
        reqbuf[len] = 0;
        if (req->v3.version == 3)
        {
            memcpy(&sa_un, &req->v3.sock, sizeof(struct sockaddr_un));
            printlog(INFO, "Request for v3 version");
            int fd_data = add_port(fd, &sa_un, DATA);

            send_ctl_message(MSG_PORT_ADD, &fd_data, sizeof(fd_data));
        }
        else if (req->v3.version > 2 || req->v3.version == 2)
        {
            printlog(ERROR, "Request for a version %d port, which this "
                            "vde_switch doesn't support",
                     req->v3.version);
        }
        else
        {
            memcpy(&sa_un, &req->v1.u.new_control.name, sizeof(struct sockaddr_un));
            printlog(INFO, "Request for v1 version");
        }
    }
    return 0;
}

int ctl_plane_loop()
{
    epoll_fds *efd = &ctl_plane_fds;

    int n;
    
    while (1)
    {
        n = epoll_wait(efd->epoll_fd, efd->events, efd->nevents, -1);

        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            printlog(ERROR, "epoll_wait() failed: %s", strerror(errno));
            return -1;
        }

        printlog(DEBUG, "epoll_wait() returned %d events", n);

        for (int i = 0; i < n; i++)
        {
            int fd = LO(efd->events[i].data.u64);
            int type = HI(efd->events[i].data.u64);

            printlog(DEBUG, "Event %d: fd=%d, events=%d", i, fd, type);

            switch (type)
            {
            case CTL_LISTEN:
                if (accept_new_ctl_conn(fd) < 0)
                {
                    printlog(ERROR, "Failed to accept new control connection on fd %d", fd);
                }
                break;
            case CTL_WD:
                if (handle_wd_req(fd) < 0)
                {
                    printlog(ERROR, "Failed to handle WD request on fd %d", fd);
                }
                break;
            default:
                printlog(ERROR, "[CTL] Unknown event type %d", type);
                break;
            }
        }
    }
}