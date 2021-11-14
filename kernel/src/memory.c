#include "memory.h"
#include "paging.h"

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

void* memset(void *dest, char val, size_t len)
{
    for(uint64_t i = 0; i < len; i++) {
        *(uint8_t*)((uint64_t)dest + i) = 0;
    }

    /*
    unsigned char *ptr = dest;
    while (len-- > 0)
    {
        *ptr = val;
        ptr++;
    }
    */
    return dest;
}

void* memzero(void *dest, size_t len)
{
    return memset(dest, 0, len);
}

int memcmp(const void *string1, const void *string2, size_t length)
{
    const unsigned char *s1 = string1, *s2 = string2;
    for(size_t i = 0; i < length; i++)
    {
        if (s1[i] < s2[i]) return -1;
        if (s1[i] > s2[i]) return 1;
    }
    return 0;
}

void* memcpy(void *dest, const void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    while (len--)
        *d++ = *s++;
    return dest;
}

void* memrcpy(void *dest, const void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    while (len--)
        *d++ = *(s + len);
    return dest;
}