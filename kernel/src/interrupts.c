#include "interrupts.h"
#include "types.h"
#include "panic.h"

#include <stdint.h>

__attribute__((interrupt)) void int_handler_pagefault(struct interrupt_frame *frame)
{
    panic("page fault detected");
    while(true);
}

__attribute__((interrupt)) void int_handler_double_fault(struct interrupt_frame *frame)
{
    panic("double fault detected");
    while(true);
}

__attribute__((interrupt)) void int_handler_general_protection(struct interrupt_frame *frame)
{
    panic("general protection fault detected");
    while(true);
}
