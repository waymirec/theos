#pragma once

#include "font.h"
#include "efimem.h"
#include "types.h"
#include "acpi.h"
#include "display.h"

typedef struct {
    framebuffer_t *framebuffer;
    psf1_font_t *font;
    memory_info_t *memory_info;
    rsdp_descriptor_t *rootSystemDescriptionPointer;
} boot_info_t;
