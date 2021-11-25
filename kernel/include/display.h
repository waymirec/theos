#pragma once

#include <stddef.h>

typedef struct {
    void *base_address;
    size_t buffer_size;
    unsigned int horizontal_resolution;
    unsigned int vertical_resolution;
    unsigned int pixels_per_scan_line;
} framebuffer_t;
