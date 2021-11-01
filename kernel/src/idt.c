#include "idt.h"
#include "interrupts.h"
#include "pageframe_allocator.h"
#include "terminal.h"
#include "string.h"

idt_descriptor_t _idtr;

void idt_init()
{
    _idtr.limit = (sizeof(idt_entry_t) * MAX_NUM_IDT_ENTRIES) - 1;
    _idtr.base = (uint64_t)pageframe_request();
    memzero((void *)_idtr.base, _idtr.limit);

    idt_entry_t *idt_entry;
    
    uint64_t pfhandler = (uint64_t)int_handler_pagefault;
    idt_entry = (idt_entry_t *)(_idtr.base + (0xE * sizeof(idt_entry_t)));
    idt_entry->base_low = IDT_BASE_LOW(pfhandler);
    idt_entry->selector = 0x08; // kernel code segment selector
    idt_entry->flags = IDT_FLAGS_INTERRUPT_GATE;
    idt_entry->base_middle = IDT_BASE_MIDDLE(pfhandler);
    idt_entry->base_high = IDT_BASE_HIGH(pfhandler);

    uint64_t dfhandler = (uint64_t)int_handler_double_fault;
    idt_entry = (idt_entry_t *)(_idtr.base + (0x08 * sizeof(idt_entry_t)));
    idt_entry->base_low = IDT_BASE_LOW(dfhandler);
    idt_entry->selector = 0x08; // kernel code segment selector
    idt_entry->flags = IDT_FLAGS_INTERRUPT_GATE;
    idt_entry->base_middle = IDT_BASE_MIDDLE(dfhandler);
    idt_entry->base_high = IDT_BASE_HIGH(dfhandler);

    uint64_t gpfhandler = (uint64_t)int_handler_general_protection;
    idt_entry = (idt_entry_t *)(_idtr.base + (0x0D * sizeof(idt_entry_t)));
    idt_entry->base_low = IDT_BASE_LOW(gpfhandler);
    idt_entry->selector = 0x08; // kernel code segment selector
    idt_entry->flags = IDT_FLAGS_INTERRUPT_GATE;
    idt_entry->base_middle = IDT_BASE_MIDDLE(gpfhandler);
    idt_entry->base_high = IDT_BASE_HIGH(gpfhandler);

    load_idt(&_idtr);
}
