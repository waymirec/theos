#include "pageframe_allocator.h"
#include <stddef.h>

#define PAGE(address) ((uint64_t)address / PAGE_SIZE)
#define ADDRESS(index) ((void *)(i * PAGE_SIZE))

uint64_t _memory_free;
uint64_t _memory_reserved;
uint64_t _memory_used;
bool _initialized;
bitmap_t _bitmap;
uint64_t _bitmap_index;

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

// private functions
static void __reserve_page(void *address);
static void __reserve_pages(void *address, size_t page_count);
static void __unreserve_page(void *address);
static void __unreserve_pages(void *address, size_t page_count);

void pageframe_allocator_init(memory_info_t *memory_info)
{
    if (_initialized) return;

    uint64_t entries = memory_info->memory_map_size / memory_info->memory_map_descriptor_size;
    void *largest_free_seg = NULL;
    size_t largest_free_size = 0;

    for (int i = 0; i < entries; i++) {
        efi_memory_descriptor_t *desc = (efi_memory_descriptor_t *)((uint64_t)memory_info->memory_map + (i * memory_info->memory_map_descriptor_size));
        if (desc->type == EFI_CONVENTIONAL_MEMORY_TYPE_INDEX) {
            size_t size = desc->page_count * PAGE_SIZE;
            if (size > largest_free_size) {
                largest_free_seg = desc->physical_address;
                largest_free_size = size;
            }
        }
    }

    uint64_t total_system_memory = system_memory_size(memory_info);
    _memory_free = total_system_memory;

    uint64_t bitmap_size = (total_system_memory / PAGE_SIZE / 8) + 1;
    bitmap_init(&_bitmap, bitmap_size, largest_free_seg);
    _bitmap_index = (uint64_t)largest_free_seg / PAGE_SIZE;

    __reserve_pages(0, (total_system_memory / PAGE_SIZE) + 1);
    for (int i=0; i < entries; i++) {
        efi_memory_descriptor_t *desc = (efi_memory_descriptor_t *)((uint64_t)memory_info->memory_map + (i * memory_info->memory_map_descriptor_size));
        if (desc->type == EFI_CONVENTIONAL_MEMORY_TYPE_INDEX) {
            __unreserve_pages(desc->physical_address, desc->page_count);
        }
    }
    __reserve_pages(0, 0x100);

    // lock kernel pages
    uint64_t kernel_size = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
    uint64_t kernel_page_count = ((uint64_t)kernel_size / PAGE_SIZE) + 1;
    pageframe_nlock(&_KernelStart, kernel_page_count);

    // lock bitmap pages
    pageframe_nlock(_bitmap.buffer, (_bitmap.size / PAGE_SIZE) + 1);

    _initialized = true;
}

bool pageframe_free(void *address)
{
    uint64_t page = PAGE(address);
    if (bitmap_check(&_bitmap, page) == false) return address;
    if (bitmap_clear(&_bitmap, page)) {
        _memory_free += PAGE_SIZE;
        _memory_used -= PAGE_SIZE;
        return true;
    }
    return false;
}

void pageframe_nfree(void *address, size_t page_count)
{
    for (int i = 0; i < page_count; i++)
    {
        pageframe_free((void *)((uint64_t)address + (i * PAGE_SIZE)));
    }
}

bool pageframe_lock(void *address)
{
    uint64_t page = PAGE(address);
    if (bitmap_check(&_bitmap, page) == true) return address;
    if (bitmap_set(&_bitmap, page)) {
        _memory_free -= PAGE_SIZE;
        _memory_used += PAGE_SIZE;
        return true;
    }
    return false;
}

void pageframe_nlock(void *address, size_t page_count)
{
    for (int i = 0; i < page_count; i++)
    {
        pageframe_lock((void *)((uint64_t)address + (i * PAGE_SIZE)));
    }
}

void* pageframe_request(void)
{
    for (size_t i = _bitmap_index; i < _bitmap.size * 8; i++) {
        if (bitmap_check(&_bitmap, i) == true) continue;
        _bitmap_index = i+1;
        return pageframe_lock(ADDRESS(i)) ? ADDRESS(i) : NULL;
    }

    if(_bitmap_index > 0) {
        for(size_t i= 0; i < _bitmap_index; i++) {
            if (bitmap_check(&_bitmap, i) == true) continue;
            _bitmap_index = i+1;
            return pageframe_lock(ADDRESS(i)) ? ADDRESS(i) : NULL;
        }
    }

    return NULL; // perform page swap
}

uint64_t pageframe_memory_free(void)
{
    return _memory_free;
}

uint64_t pageframe_memory_used(void)
{
    return _memory_used;
}

uint64_t pageframe_memory_reserved(void)
{
    return _memory_reserved;
}

static void __reserve_page(void *address)
{
    uint64_t page = PAGE(address);
    if (bitmap_check(&_bitmap, page) == true) return;
    bitmap_set(&_bitmap, page);
    _memory_free -= PAGE_SIZE;
    _memory_reserved += PAGE_SIZE;
}

static void __reserve_pages(void *address, size_t page_count)
{
    for (int i = 0; i < page_count; i++)
    {
        __reserve_page((void *)((uint64_t)address + (i * PAGE_SIZE)));
    }
}

static void __unreserve_page(void *address)
{
    uint64_t page = PAGE(address);
    if (bitmap_check(&_bitmap, page) == false) return;
    bitmap_clear(&_bitmap, page);
    _memory_free += PAGE_SIZE;
    _memory_reserved -= PAGE_SIZE;
}

static void __unreserve_pages(void *address, size_t page_count)
{
    for (int i = 0; i < page_count; i++)
    {
        __unreserve_page((void *)((uint64_t)address + (i * PAGE_SIZE)));
    }
}