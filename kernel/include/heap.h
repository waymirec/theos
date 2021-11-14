#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct heap_hdr_t heap_hdr_t;
struct heap_hdr_t {
    size_t length;
    heap_hdr_t *next;
    heap_hdr_t *prev;
    bool free;
};

void heap_init(void *address, size_t pages);
void* heap_alloc(size_t size);
void heap_free(void *address);
