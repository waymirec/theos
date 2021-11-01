#pragma once

#include "font.h"
#include "efimem.h"
#include "types.h"

typedef struct {
    void *base_address;
    size_t buffer_size;
    unsigned int horizontal_resolution;
    unsigned int vertical_resolution;
    unsigned int pixels_per_scan_line;
} framebuffer_t;

typedef struct {
    framebuffer_t *framebuffer;
    psf1_font_t *font;
    memory_info_t *memory_info;
} boot_info_t;
