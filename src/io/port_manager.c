#include <stdlib.h>
#include <stdio.h>
#include "port.h"
#include "polling.h"
#include "logging.h"
#include "utils.h"
#include "switch.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

extern int ctl_pipe[2];
extern epoll_fds ctl_plane_fds;
extern char ctl_socket[PATH_MAX];

int portc = 1;
endpoint *endpoints = NULL;

fd_table_t* fdt;

void init_ports(int numports)
{
    port_init(numports);
    fdt = fd_table_create();

    // robe...

    return;
}

int add_port(int fd, struct sockaddr_un *sun_out, int type)
{
    int fd_data = -1;
    struct sockaddr_un sun_in;
    const size_t max_ctl_sock_len = sizeof(sun_in.sun_path) - 8;
    memset(&sun_in, 0, sizeof(sun_in));

    if ((fd_data = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        printlog(ERROR, "socket() failed");
        return -1;
    }
    if (fcntl(fd_data, F_SETFL, O_NONBLOCK) < 0)
    {
        printlog(WARNING, "fcntl - setting O_NONBLOCK %s", strerror(errno));
        close(fd_data);
        return -1;
    }

    if (connect(fd_data, (struct sockaddr *)sun_out, sizeof(struct sockaddr_un)) < 0)
    {
        printlog(ERROR, "connect() failed: %s", strerror(errno));
        close(fd_data);
        return -1;
    }

    sun_in.sun_family = AF_UNIX;
    endpoint *ep = alloc_endpoint(portc++, fd_data, getuid(), NULL, NULL);
    if (endpoints == NULL)
    {
        endpoints = ep;
    }
    else
    {
        ep->next = endpoints;
        endpoints = ep;
    }

    printlog(DEBUG, "Trying to bind to %s/%03d.%d", ctl_socket, ep->port, fd_data);

    snprintf(sun_in.sun_path, sizeof(sun_in.sun_path), "%s/%03d.%d", ctl_socket, ep->port, fd_data);

    if ((unlink(sun_in.sun_path) < 0 && errno != ENOENT) || bind(fd_data, (struct sockaddr *)&sun_in, sizeof(sun_in)) < 0)
    {
        printlog(ERROR, "bind() failed: %s", strerror(errno));
        close(fd_data);
        return -1;
    }
    int n = write(fd, &sun_in, sizeof(sun_in));

    if (n < 0)
    {
        printlog(ERROR, "write() failed");
        close(fd_data);
        return -1;
    }

    fd_table_insert(fdt, fd_data, ep);
    
    return fd_data;
}

void remove_port(int portno)
{
    port *p = get_port(portno);
    if (p == NULL)
    {
        printlog(ERROR, "Invalid port number %d", portno);
        return;
    }
    endpoint *ep = p->ep;
    while (ep != NULL)
    {
        remove_fd(&ctl_plane_fds, ep->fd_data);
        endpoint *next = ep->next;
        free_endpoint(ep);
        ep = next;
    }
    free_port(p);
    printlog(INFO, "Port %d removed and detached from polling", portno);
}
