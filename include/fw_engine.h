#ifndef FW_ENGINE_H
#define FW_ENGINE_H

struct packet;

int fw_engine_init(int num_threads);
int dispatch_packet(int fd);
int flood_packet(packet *p, int origin_port);
int send_packet(packet* p, int port);

#define IS_BROADCAST(addr) (((addr)[0] & 1) == 1)

#endif