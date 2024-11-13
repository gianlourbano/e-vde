#ifndef FLOW_TABLE_H
#define FLOW_TABLE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_FIELD_LENGTH 128
#define MAX_ACTION_LENGTH 64
#define HASH_TABLE_SIZE 10007 // A prime number for better distribution

typedef struct
{
    uint8_t field[MAX_FIELD_LENGTH];
    uint8_t mask[MAX_FIELD_LENGTH];
    uint16_t length;
} flow_key;

typedef struct
{
    uint8_t action[MAX_ACTION_LENGTH];
    uint16_t action_length;
} FlowAction;

typedef struct flow_entry
{
    flow_key key;
    FlowAction action;
    struct flow_entry *next;     // For hash table collision handling
    struct flow_entry *child[2]; // For trie structure (0 and 1 branches)
} flow_entry;

typedef struct
{
    flow_entry *hash_table[HASH_TABLE_SIZE];
    flow_entry *trie_root;
} flow_table;

// Function prototypes
flow_table *flow_table_create();
void flow_table_destroy(flow_table *table);
int flow_table_insert(flow_table *table, const flow_key *key, const FlowAction *action);
flow_entry *flow_table_lookup(flow_table *table, const flow_key *key);
int flow_table_delete(flow_table *table, const flow_key *key);
void flow_table_print(flow_table *table); // For debugging

#endif // FLOW_TABLE_H