#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include "port.h"
#include "packet.h"
#include "logging.h"
#include "memory.h"

static int NUMPORTS = 24;

static arena *port_arena;

void port_init(int numports)
{
    if ((NUMPORTS = numports) < 1)
    {
        printlog(ERROR, "Invalid number of ports\n");
        exit(1);
    }

    port_arena = arena_alloc(sizeof(port) * NUMPORTS);
}

void debug_ports()
{
    for (int i = 0; i < NUMPORTS; i++)
    {
        port *p = ((port *)port_arena->ptr + i);
        if (p->flag == PORT_FLAG_UP)
            printlog(DEBUG, "Port %d: %d", i, p->flag);
    }
}

port *alloc_port(int portno)
{
    if (portno < 0 || portno >= NUMPORTS)
    {
        printlog(ERROR, "Invalid port number\n");
        exit(1);
    }

    // if portno = 0, find the first available port
    if (portno == 0)
    {
        for (int i = 0; i < NUMPORTS; i++)
        {
            port *p = ((port *)port_arena->ptr + i);
            if (p->flag == PORT_FLAG_DOWN)
            {
                portno = i;
                break;
            }
        }
    }

    port *p = ((port *)port_arena->ptr + portno);
    if (p->flag == PORT_FLAG_DOWN)
    {
        p->flag = PORT_FLAG_UP;
        p->ep = NULL;
        p->fd_ctl = -1;
        p->sender = NULL;
        p->receiver = NULL;
        p->user = 0;
        p->group = 0;
        p->curuser = 0;
        p->vlanuntag = 0;
    }
    else
    {
        return NULL;
    }

    return p;
}

inline port *get_port(int portno)
{
    if (portno < 0 || portno >= NUMPORTS)
    {
        printlog(ERROR, "Invalid port number: %d\n", portno);
        exit(1);
    }

    port *p = ((port *)port_arena->ptr + portno);

    if(p->flag == PORT_FLAG_DOWN || p->ep == NULL) {
        return NULL;
    }

    return p;
}

void free_port(port *p)
{
    p->flag = PORT_FLAG_DOWN;
    if (p->ep != NULL)
    {
        endpoint *e = p->ep;
        while (e != NULL)
        {
            endpoint *next = e->next;
            free(e->descr);
            free(e);
            e = next;
        }
    }
    p->ep = NULL;
    p->fd_ctl = -1;
    p->sender = NULL;
    p->receiver = NULL;
    p->user = 0;
    p->group = 0;
    p->curuser = 0;
    p->vlanuntag = 0;
}

void port_cleanup()
{
    arena_free(port_arena);
}

endpoint *alloc_endpoint(int portno, uint64_t fd_data, uid_t user, sender_t sender, receiver_t receiver)
{

    port *p;

    if ((p = alloc_port(portno)) == NULL)
    {
        printlog(WARNING, "Port already in use\n");
        return NULL;
    }

    endpoint *ep = malloc(sizeof(endpoint));

    ep->port = portno;
    ep->fd_data = fd_data;
    ep->descr = NULL;
    ep->next = NULL;

    p->ep = ep;
    p->fd_ctl = -1;
    p->sender = sender;
    p->receiver = receiver;
    p->user = user;

    /*
    TODO: add endpoint to endpoint list
    */

    return ep;
}

void free_endpoint(endpoint *e)
{
}

void set_port_state(int portno, int state)
{
    port *p = get_port(portno);
    if (p == NULL)
    {
        printlog(ERROR, "Invalid port number %d", portno);
        return;
    }
    p->flag = state;
    printlog(INFO, "Port %d state set to %d", portno, state);
}