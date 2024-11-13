#ifndef PORT_H
#define PORT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <sys/un.h>
#include "packet.h"

typedef struct endpoint {
    int port;

    uint64_t fd_data;
    char* descr;

    struct endpoint* next;
} endpoint;

#define PORT_FLAG_DOWN 0
#define PORT_FLAG_UP 1

typedef int (* sender_t)(int , int , packet* , int , int);
typedef int (* receiver_t)(int , int , packet* , int , int);

typedef struct port {
    int flag;

    endpoint* ep;
    int fd_ctl;

    sender_t sender;
    receiver_t receiver;

    uid_t user;
    gid_t group;
    uid_t curuser;

    int vlanuntag;


} port;

#define FD_TABLE_INITIAL_SIZE 1024

typedef struct fd_table {
    endpoint** endpoints;  // Array of pointers to endpoints
    size_t capacity;      // Current array size
    _Atomic size_t max_fd;  // Highest fd seen
} fd_table_t;

// Add to existing port.h declarations
fd_table_t* fd_table_create(void);
void fd_table_destroy(fd_table_t* table);
int fd_table_insert(fd_table_t* table, int fd, endpoint* ep);
endpoint* fd_table_lookup(fd_table_t* table, int fd);
void fd_table_remove(fd_table_t* table, int fd);


// endpoint methods

endpoint* alloc_endpoint(int portno, uint64_t fd_data, uid_t user, sender_t sender, receiver_t receiver);
void free_endpoint(endpoint* e);

// port methods

void port_init(int numports);
port* alloc_port(int portno);
port* get_port(int portno);
void free_port(port* p);
void port_cleanup();

int add_port(int fd, struct sockaddr_un* sun_out, int type);

#endif