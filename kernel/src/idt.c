#include "idt.h"
#include "interrupt_handlers.h"
#include "pageframe_allocator.h"
#include "string.h"

idt_descriptor_t _idtr;

void idt_init()
{
    _idtr.limit = (sizeof(idt_entry_t) * MAX_NUM_IDT_ENTRIES) - 1;
    _idtr.base = (uint64_t)pageframe_request();
    memzero((void *)_idtr.base, _idtr.limit);

    set_idt_gate(int_handler_pagefault, 0xE, 0x08, IDT_FLAGS_INTERRUPT_GATE);
    set_idt_gate(int_handler_double_fault, 0x08, 0x08, IDT_FLAGS_INTERRUPT_GATE);
    set_idt_gate(int_handler_general_protection, 0x0D, 0x08, IDT_FLAGS_INTERRUPT_GATE);
    set_idt_gate(int_handler_keyboard, 0x21, 0x08, IDT_FLAGS_INTERRUPT_GATE);
    set_idt_gate(int_handler_mouse, 0x2C, 0x08, IDT_FLAGS_INTERRUPT_GATE);
    set_idt_gate(int_handler_pit, 0x20, 0x08, IDT_FLAGS_INTERRUPT_GATE);

    load_idt(&_idtr);
}

void set_idt_gate(void *handler, uint8_t offset, uint8_t selector, uint8_t flags)
{
    uint64_t h = (uint64_t)handler;
    idt_entry_t *entry = (idt_entry_t *)(_idtr.base + (offset * sizeof(idt_entry_t)));
    entry->base_low = IDT_BASE_LOW(h);
    entry->selector = selector;
    entry->flags = flags;
    entry->base_middle = IDT_BASE_MIDDLE(h);
    entry->base_high = IDT_BASE_HIGH(h);
}