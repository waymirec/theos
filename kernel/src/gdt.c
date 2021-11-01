#include "gdt.h"

extern void load_gdt(gdt_descriptor_t *gdt_descriptor);

gdt_descriptor_t gdt_ptr;

__attribute__((aligned(0x1000)))
gdt_t default_gdt = {
    {.limit_low=0, .base_low=0, .base_middle=0, .access=0x00, .granularity=0x00, .base_high=0}, // NULL
    {.limit_low=0, .base_low=0, .base_middle=0, .access=0x9A, .granularity=0xA0, .base_high=0}, // KERNEL CODE
    {.limit_low=0, .base_low=0, .base_middle=0, .access=0x92, .granularity=0xA0, .base_high=0}, // KERNEL DATA
    {.limit_low=0, .base_low=0, .base_middle=0, .access=0x00, .granularity=0x00, .base_high=0}, // USER NULL
    {.limit_low=0, .base_low=0, .base_middle=0, .access=0x9A, .granularity=0xA0, .base_high=0}, // USER CODE
    {.limit_low=0, .base_low=0, .base_middle=0, .access=0x92, .granularity=0xA0, .base_high=0}  // USER DATA
};

void gdt_init()
{
    gdt_ptr.size = sizeof(gdt_t) - 1;
    gdt_ptr.base = (uint64_t)&default_gdt;
    load_gdt(&gdt_ptr);
}