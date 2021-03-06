#include "memory.h"
#include "paging.h"
#include "string.h"

uint64_t system_memory_size(memory_info_t *memory_info)
{
    static uint64_t memory_bytes = 0;
    if (memory_bytes > 0) return memory_bytes;

    uint64_t memory_map_entries = memory_info->memory_map_size / memory_info->memory_map_descriptor_size;
    for (int i=0; i<memory_map_entries; i++)
    {
        efi_memory_descriptor_t *desc = (efi_memory_descriptor_t *)((uint64_t)memory_info->memory_map + (i * memory_info->memory_map_descriptor_size));
        memory_bytes += desc->page_count * PAGE_SIZE;
    }

    return memory_bytes;
}
