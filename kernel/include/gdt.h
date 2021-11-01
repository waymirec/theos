 #pragma once

#include "types.h"

#include <stdint.h>

typedef struct {
     uint16_t size;
     uint64_t base;
}__attribute__((packed)) gdt_descriptor_t;

typedef struct {
     uint16_t limit_low;
     uint16_t base_low;
     uint8_t base_middle;
     uint8_t access;
     uint8_t granularity;
     uint8_t base_high;
}__attribute__((packed)) gdt_entry_t;

typedef struct {
     gdt_entry_t null;                //0x00
     gdt_entry_t kernel_code;         //0x08
     gdt_entry_t kernel_data;         //0x10
     gdt_entry_t user_null;
     gdt_entry_t user_code;
     gdt_entry_t user_data;
}__attribute__((packed,aligned(0x1000))) gdt_t;

void gdt_init();