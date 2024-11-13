#ifndef TOEPLITZ_H
#define TOEPLITZ_H

#include <stdint.h>
#include <stddef.h>

struct packet;

void toeplitz_init();
uint32_t toeplitz_hash(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len);

uint32_t compute_packet_hash(const uint8_t *packet_data, size_t packet_len);

#endif