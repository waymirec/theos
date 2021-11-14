#pragma once
#include "types.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    size_t size;
    uint8_t *buffer;
} bitmap_t;

void bitmap_init(bitmap_t *bitmap, size_t size, void *buffer);
bool bitmap_check(bitmap_t *bitmap, uint64_t index);
bool bitmap_set(bitmap_t *bitmap, uint64_t index);
bool bitmap_clear(bitmap_t *bitmap, uint64_t index);