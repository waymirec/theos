#pragma once

#include "font.h"
#include "efimem.h"
#include "types.h"
#include "acpi.h"

#include <stdint.h>
#include <stddef.h>

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
    rsdp_descriptor_t *rootSystemDescriptionPointer;
} boot_info_t;
