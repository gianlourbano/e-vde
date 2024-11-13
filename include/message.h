#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>

/**
 * Communication from control plane to data plane
 * 
 */

enum MESSAGE_TYPE {
    MSG_PORT_UP,
    MSG_PORT_DOWN,
    MSG_PORT_ADD,
    MSG_PORT_DEL
};

int send_ctl_message(int type, void* data, size_t len);
int close_ctl_pipe();

#endif