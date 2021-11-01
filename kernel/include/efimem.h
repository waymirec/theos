#pragma once

#include "types.h"

typedef struct {
    uint32_t type;
    void *physical_address;
    void *virtual_address;
    uint64_t page_count;
    uint64_t attributes;
} efi_memory_descriptor_t;

typedef struct {
    efi_memory_descriptor_t *memory_map;
    uint64_t memory_map_size;
    uint64_t memory_map_key;
    uint64_t memory_map_descriptor_size;
    uint32_t memory_map_descriptor_version;
} memory_info_t;

extern const char *EFI_MEMORY_TYPE_STRINGS[];

#define EFI_CONVENTIONAL_MEMORY_TYPE_INDEX 7