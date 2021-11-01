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

    uint64_t pfhandler = (uint64_t)pagefault_handler;
    idt_entry_t *int_pagefault = (idt_entry_t *)(_idtr.base + (0xE * 16));
    int_pagefault->base_low = IDT_BASE_LOW(pfhandler);
    int_pagefault->selector = 0x08; // kernel code segment selector
    int_pagefault->flags = IDT_FLAGS_INTERRUPT_GATE;
    int_pagefault->base_middle = IDT_BASE_MIDDLE(pfhandler);
    int_pagefault->base_high = IDT_BASE_HIGH(pfhandler);
    char idt_terminal_buffer[128];
    terminal_nprintln(2, "sizeof(idt_entry_t): ", uint_to_string(sizeof(*int_pagefault), idt_terminal_buffer));
    load_idt(&_idtr);
}
