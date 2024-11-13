#include "port.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

fd_table_t* fd_table_create(void) {
    fd_table_t* table = calloc(1, sizeof(fd_table_t));
    table->capacity = FD_TABLE_INITIAL_SIZE;
    table->endpoints = calloc(table->capacity, sizeof(endpoint*));
    atomic_store(&table->max_fd, 0);
    return table;
}

static int fd_table_resize(fd_table_t* table, size_t new_fd) {
    if (new_fd >= table->capacity) {
        size_t new_size = table->capacity * 2;
        while (new_size <= new_fd) new_size *= 2;
        
        endpoint** new_array = realloc(table->endpoints, new_size * sizeof(endpoint*));
        if (!new_array) return -1;
        
        memset(new_array + table->capacity, 0, 
               (new_size - table->capacity) * sizeof(endpoint*));
        
        table->endpoints = new_array;
        table->capacity = new_size;
    }
    return 0;
}

int fd_table_insert(fd_table_t* table, int fd, endpoint* ep) {
    if (fd_table_resize(table, fd) < 0) return -1;
    
    table->endpoints[fd] = ep;
    
    size_t current_max = atomic_load(&table->max_fd);
    while (fd > current_max) {
        if (atomic_compare_exchange_weak(&table->max_fd, &current_max, fd)) {
            break;
        }
    }
    return 0;
}

endpoint* fd_table_lookup(fd_table_t* table, int fd) {
    if (fd < 0 || fd >= table->capacity) return NULL;
    return table->endpoints[fd];
}

void fd_table_remove(fd_table_t* table, int fd) {
    if (fd >= 0 && fd < table->capacity) {
        table->endpoints[fd] = NULL;
    }
}