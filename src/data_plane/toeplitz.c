#include <stdint.h>
#include <string.h>

#include "logging.h"
#include "packet.h"
#include "toeplitz.h"

/**
 * Toeplitz hash based on ethernet packet header
 *
 * We use this to determine which thread to send the packet to
 *
 */

// Define the Toeplitz hash key
static const uint8_t toeplitz_key[] = {
    0x6d, 0x5a, 0x56, 0xfa,
    0x6d, 0x5a, 0x56, 0xfa,
    // Extend the key as needed
};

void toeplitz_init()
{
}

// Toeplitz hash function
uint32_t toeplitz_hash(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len)
{
    uint32_t result = 0;
    size_t bits = data_len * 8;

    for (size_t i = 0; i < bits; i++)
    {
        if (data[i / 8] & (1 << (7 - (i % 8))))
        {
            result ^= ((uint32_t)key[i % key_len] << 24) |
                      ((uint32_t)key[(i + 1) % key_len] << 16) |
                      ((uint32_t)key[(i + 2) % key_len] << 8) |
                      ((uint32_t)key[(i + 3) % key_len]);
        }
    }

    return result;
}

// Compute hash based on source/dest MAC addresses
uint32_t compute_packet_hash(const uint8_t *packet_data, size_t packet_len)
{
    // Ensure packet is large enough for Ethernet header
    if (packet_len < 14)
    {
        return 0;
    }

    // Extract MAC addresses
    const uint8_t *dest_mac = packet_data;
    const uint8_t *src_mac = packet_data + 6;

    // Build data: src_mac + dest_mac
    uint8_t data[12];
    memcpy(data, src_mac, 6);
    memcpy(data + 6, dest_mac, 6);

    // Compute Toeplitz hash
    return toeplitz_hash(toeplitz_key, sizeof(toeplitz_key), data, sizeof(data));
}