#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <stdatomic.h>

#define HASH_TABLE_SIZE 4096
#define HASH_TABLE_MASK (HASH_TABLE_SIZE - 1)
#define MAC_SIZE 6
#define ENTRY_TTL 300 // 5 minutes in seconds


typedef struct hash_entry
{
    uint8_t mac[MAC_SIZE];
    uint16_t vlan;
    int port;
    atomic_uint_least64_t last_seen; // Changed from atomic_time_t
    struct hash_entry *next;
    _Atomic int valid;
} hash_entry_t;

typedef struct
{
    hash_entry_t *buckets[HASH_TABLE_SIZE];
    atomic_size_t size;
} mac_table_t;

mac_table_t* mac_table_create();
int mac_table_insert(mac_table_t *table, const uint8_t * mac, uint16_t vlan, int port);
int mac_table_lookup(mac_table_t *table, const uint8_t * mac, uint16_t vlan);

#endif