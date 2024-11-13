#include "flow_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static uint32_t hash_function(const flow_key *key)
{
    uint32_t hash = 5381;
    for (int i = 0; i < key->length; i++)
    {
        hash = ((hash << 5) + hash) + key->field[i];
    }
    return hash % HASH_TABLE_SIZE;
}

flow_table *flow_table_create()
{
    flow_table *table = calloc(1, sizeof(flow_table));
    if (!table)
        return NULL;
    return table;
}

void flow_table_destroy(flow_table *table)
{
    if (!table)
        return;

    // Free hash table entries
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        flow_entry *entry = table->hash_table[i];
        while (entry)
        {
            flow_entry *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    // TODO: Implement trie destruction (recursive function)

    free(table);
}

static flow_entry *create_flow_entry(const flow_key *key, const FlowAction *action)
{
    flow_entry *entry = calloc(1, sizeof(flow_entry));
    if (!entry)
        return NULL;

    memcpy(&entry->key, key, sizeof(flow_key));
    memcpy(&entry->action, action, sizeof(FlowAction));

    return entry;
}

int flow_table_insert(flow_table *table, const flow_key *key, const FlowAction *action)
{
    if (!table || !key || !action)
        return -1;

    uint32_t hash = hash_function(key);
    flow_entry *new_entry = create_flow_entry(key, action);
    if (!new_entry)
        return -1;

    // Insert into hash table
    new_entry->next = table->hash_table[hash];
    table->hash_table[hash] = new_entry;

    // Insert into trie
    flow_entry **node = &table->trie_root;
    for (int i = 0; i < key->length * 8; i++)
    {
        int bit = (key->field[i / 8] >> (7 - (i % 8))) & 1;
        if (!*node)
        {
            *node = calloc(1, sizeof(flow_entry));
            if (!*node)
                return -1;
        }
        node = &((*node)->child[bit]);
    }
    *node = new_entry;

    return 0;
}

flow_entry *flow_table_lookup(flow_table *table, const flow_key *key)
{
    if (!table || !key)
        return NULL;

    // Try exact match in hash table first
    uint32_t hash = hash_function(key);
    flow_entry *entry = table->hash_table[hash];
    while (entry)
    {
        if (memcmp(&entry->key, key, sizeof(flow_key)) == 0)
        {
            return entry;
        }
        entry = entry->next;
    }

    // If not found, try wildcard match in trie
    flow_entry *node = table->trie_root;
    flow_entry *last_match = NULL;
    for (int i = 0; i < key->length * 8 && node; i++)
    {
        int bit = (key->field[i / 8] >> (7 - (i % 8))) & 1;
        if (node->action.action_length > 0)
        {
            last_match = node; // Update last match if this node has an action
        }
        node = node->child[bit];
    }

    return last_match ? last_match : node;
}

int flow_table_delete(flow_table *table, const flow_key *key)
{
    if (!table || !key)
        return -1;

    uint32_t hash = hash_function(key);
    flow_entry **entry = &table->hash_table[hash];
    while (*entry)
    {
        if (memcmp(&(*entry)->key, key, sizeof(flow_key)) == 0)
        {
            flow_entry *to_delete = *entry;
            *entry = (*entry)->next;
            free(to_delete);
            return 0;
        }
        entry = &(*entry)->next;
    }

    // TODO: Implement deletion from trie (more complex, might need parent pointers)

    return -1; // Not found
}

void flow_table_print(flow_table *table)
{
    if (!table)
        return;

    printf("Flow Table Contents:\n");
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        flow_entry *entry = table->hash_table[i];
        while (entry)
        {
            printf("Hash %d: Key length %d, Action length %d\n",
                   i, entry->key.length, entry->action.action_length);
            entry = entry->next;
        }
    }

    // TODO: Implement trie printing (recursive function)
}