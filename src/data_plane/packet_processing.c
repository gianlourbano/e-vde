#include "switch.h"
#include "ring.h"
#include "packet.h"
#include "logging.h"

#include <arpa/inet.h>

struct ip_header {
    uint8_t version_ihl;
    uint8_t dscp_ecn;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
};

int deep_packet_inspect(packet* p) {
    // inspect packet if it's ipv4

    if (p->eth.proto[0] == 0x08 && p->eth.proto[1] == 0x00) {
        printlog(INFO, "IPv4 packet");

        // inspect packet if it's tcp
        // look inside the data

        char* data = p->payload;

        struct ip_header* ip = (struct ip_header*)data;

        if (ip->protocol == 0x06) {
            printlog(INFO, "TCP packet");
        }

        // print ip src and dest
        char src_ip[INET_ADDRSTRLEN];
        char dest_ip[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &ip->src_ip, src_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &ip->dest_ip, dest_ip, INET_ADDRSTRLEN);

        printlog(INFO, "IP src: %s", src_ip);
        printlog(INFO, "IP dest: %s", dest_ip);

        // print fragment offset
        printlog(INFO, "Fragment offset: %d", ntohs(ip->flags_fragment_offset) & 0x1FFF);


    }

    return 0;
}