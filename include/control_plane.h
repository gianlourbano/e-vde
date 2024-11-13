#ifndef CONTROL_PLANE_H
#define CONTROL_PLANE_H

#include <sys/types.h>

enum CTL_REQ_TYPE {
    CTL_LISTEN,
    CTL_WD,
    DATA
};

int init_ctl_plane();
int ctl_plane_loop();

int send_ctl_message(int type, void* data, size_t len);

#endif