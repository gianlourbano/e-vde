#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <time.h>

#include "switch.h"
#include "hash_table.h"



// FNV-1a hash function
static uint32_t hash_mac(const uint8_t *mac, uint16_t vlan)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < MAC_SIZE; i++)
    {
        hash ^= mac[i];
        hash *= 16777619;
    }
    hash ^= vlan & 0xFF;
    hash ^= (vlan >> 8) & 0xFF;
    return hash & HASH_TABLE_MASK;
}

mac_table_t *mac_table_create(void)
{
    mac_table_t *table = calloc(1, sizeof(mac_table_t));
    atomic_store(&table->size, 0);
    return table;
}

int mac_table_insert(mac_table_t *table, const uint8_t *mac,
                     uint16_t vlan, int port)
{
    uint32_t idx = hash_mac(mac, vlan);
    hash_entry_t *entry = table->buckets[idx];

    // Check for existing entry
    while (entry != NULL)
    {
        if (memcmp(entry->mac, mac, MAC_SIZE) == 0 && entry->vlan == vlan)
        {
            entry->port = port;
            atomic_store(&entry->last_seen, (uint_least64_t)time(NULL));
            return true;
        }
        entry = entry->next;
    }

    // Create new entry
    entry = calloc(1, sizeof(hash_entry_t));
    memcpy(entry->mac, mac, MAC_SIZE);
    entry->vlan = vlan;
    entry->port = port;
    atomic_store(&entry->last_seen, (uint_least64_t)time(NULL));
    atomic_store(&entry->valid, true);

    // Insert at head of chain
    entry->next = table->buckets[idx];
    table->buckets[idx] = entry;
    atomic_fetch_add(&table->size, 1);

    return true;
}

int mac_table_lookup(mac_table_t *table, const uint8_t *mac,
                     uint16_t vlan)
{
    uint32_t idx = hash_mac(mac, vlan);
    hash_entry_t *entry = table->buckets[idx];
    time_t now = time(NULL);

    while (entry != NULL)
    {
        if (memcmp(entry->mac, mac, MAC_SIZE) == 0 &&
            entry->vlan == vlan &&
            atomic_load(&entry->valid))
        {

            time_t last_seen = (time_t)atomic_load(&entry->last_seen);
            if (now - last_seen > ENTRY_TTL)
            {
                atomic_store(&entry->valid, false);
                return -1;
            }
            return entry->port;
        }
        entry = entry->next;
    }

    return -1; // Not found
}

void mac_table_cleanup(mac_table_t *table)
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        hash_entry_t *entry = table->buckets[i];
        while (entry != NULL)
        {
            hash_entry_t *next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(table);
}