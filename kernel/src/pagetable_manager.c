#include "pagetable_manager.h"

#include <string.h>

#include "pageframe_allocator.h"


#define PAGE_BIT_P_PRESENT (1<<0)
#define PAGE_BIT_RW_WRITABLE (1<<1)
#define PAGE_BIT_US_USER (1<<2)
#define PAGE_XD_NX (1<<63)
#define PAGE_ADDR_MASK 0x000ffffffffff000
#define PAGE_BIT_A_ACCESSED (1<<5)
#define PAGE_BIT_D_DIRTY (1<<6)

extern void load_pml4(struct mapping_table *pml4);

void pagetable_init(pml4_t *pml4, boot_info_t *boot_info)
{
    memzero((void *)pml4, PAGE_SIZE);

    memory_info_t *memory_info = boot_info->memory_info;

    //identity mapping
    uint64_t memory_size = system_memory_size(memory_info); //67108864;
    pagetable_identity_map(pml4, (void *)0, memory_size / PAGE_SIZE + 1);

    // map framebuffer in existing address space
    uint64_t framebuffer_base = (uint64_t)boot_info->framebuffer->base_address;
    uint64_t framebuffer_size = (uint64_t)boot_info->framebuffer->buffer_size + PAGE_SIZE; // padded just in case
    pagetable_identity_map(pml4, (void *)framebuffer_base, framebuffer_size / PAGE_SIZE + 1);
   
    // load in to CR3
    load_pml4(pml4);
}

void pagetable_map(pml4_t *pml4, void *logical_address, void *physical_address)
{
    /* flags: page is present, user readable and writable */
    int flags = PAGE_BIT_P_PRESENT | PAGE_BIT_RW_WRITABLE | PAGE_BIT_US_USER;

    int pml4_idx = ((uint64_t)logical_address >> 39) & 0x1FF;
    int pdp_idx = ((uint64_t)logical_address >> 30) & 0x1FF;
    int pd_idx = ((uint64_t)logical_address >> 21) & 0x1FF;
    int pt_idx = ((uint64_t)logical_address >> 12) & 0x1FF;

    if (!(pml4->entries[pml4_idx] & PAGE_BIT_P_PRESENT)) {
        uint64_t pdpt_alloc = (uint64_t)pageframe_request();
        memzero((void *)pdpt_alloc, PAGE_SIZE);
        pml4->entries[pml4_idx] = (pdpt_alloc & PAGE_ADDR_MASK) | flags;
        pagetable_map(pml4, (void *)pdpt_alloc, (void *)pdpt_alloc);
    }

    mapping_table_t *pdpt = (mapping_table_t *)(pml4->entries[pml4_idx] & PAGE_ADDR_MASK);
    if (!(pdpt->entries[pdp_idx] & PAGE_BIT_P_PRESENT)) {
        uint64_t pdt_alloc = (uint64_t)pageframe_request();
        memzero((void *)pdt_alloc, PAGE_SIZE);
        pdpt->entries[pdp_idx] = (pdt_alloc & PAGE_ADDR_MASK) | flags;
        pagetable_map(pml4, (void *)pdt_alloc, (void *)pdt_alloc);
    }

    mapping_table_t *pdt = (mapping_table_t *)(pdpt->entries[pdp_idx] & PAGE_ADDR_MASK);
    if (!(pdt->entries[pd_idx] & PAGE_BIT_P_PRESENT)) {
        uint64_t pt_alloc = (uint64_t)pageframe_request();
        memzero((void *)pt_alloc, PAGE_SIZE);
        pdt->entries[pd_idx] = (pt_alloc & PAGE_ADDR_MASK) | flags;
        pagetable_map(pml4, (void *)pt_alloc, (void *)pt_alloc);
    }

    mapping_table_t *pt = (mapping_table_t *)(pdt->entries[pd_idx] & PAGE_ADDR_MASK);
    if (!(pt->entries[pt_idx] & PAGE_BIT_P_PRESENT)) {
        pt->entries[pt_idx] = ((uint64_t)physical_address & PAGE_ADDR_MASK) | flags;
    }
}

void pagetable_identity_map(pml4_t *pml4, void *start, size_t page_count)
{
    for(uint64_t i = 0; i < page_count; i++) {
        uint64_t addr = (uint64_t)start + (i * PAGE_SIZE);
        pagetable_map(pml4, (void *)addr, (void *)addr);
    }
}