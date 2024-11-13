#ifndef PACKET_H
#define PACKET_H

#define ETH_ADDR_LEN 6
#define ETH_HEADER_LEN 14

typedef struct eth_header {
    unsigned char dest[ETH_ADDR_LEN];
    unsigned char src[ETH_ADDR_LEN];
    unsigned char proto[2];
} eth_header;

typedef struct eth_header_8021q {
    unsigned char dest[ETH_ADDR_LEN];
    unsigned char src[ETH_ADDR_LEN];
    unsigned char tpid[2];
    unsigned char tci[2];
    unsigned char proto[2];
} eth_header_8021q;

typedef struct packet {
    eth_header eth;
    unsigned char payload[1504];
} packet;

typedef struct tagged_packet {
    unsigned char filler[4];
    packet pkt;
} tagged_packet;

int deep_packet_inspect(packet* p);

#endif