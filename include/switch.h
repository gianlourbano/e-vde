#ifndef EVDE_SWITCH_H
#define EVDE_SWITCH_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdint.h>

#define true 1
#define false 0

#ifndef VDESTDSOCK
#define VDESTDSOCK "/var/run/vde.ctl"
#define VDETMPSOCK "/tmp/vde.ctl"
#endif

#define PATH_MAX 4096

#define DATA_BUF_SIZE 131072
#define SWITCH_MAGIC 0xfeedface
#define REQBUFLEN 256
#define ETH_ALEN 6


enum request_type
{
    REQ_NEW_CONTROL,
    REQ_NEW_PORT0
};

struct __attribute__((packed)) req_v1_new_control_s
{
    unsigned char addr[ETH_ALEN];
    struct sockaddr_un name;
};

struct request_v1
{
    uint32_t magic;
    enum request_type type;
    union
    {
        struct req_v1_new_control_s new_control;
    } u;
    char description[];
} __attribute__((packed));

struct request_v3
{
    uint32_t magic;
    uint32_t version;
    enum request_type type;
    struct sockaddr_un sock;
    char description[];
} __attribute__((packed));

union request
{
    struct request_v1 v1;
    struct request_v3 v3;
};


#endif