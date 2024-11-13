#ifndef DATA_PLANE_H
#define DATA_PLANE_H

enum DATA_REQ_TYPE {
    PACKET,
    CTL_MESSAGE
};

int init_data_plane();
void wait_for_data();

#endif