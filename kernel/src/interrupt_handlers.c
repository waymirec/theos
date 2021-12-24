#include "interrupt_handlers.h"

#include <stdint.h>
#include <stdbool.h>

#include "types.h"
#include "panic.h"
#include "io.h"
#include "8259_pic.h"
#include "irq.h"
#include "ps2_keyboard.h" // todo: refactor this
#include "ps2_mouse.h"
#include "pit.h"
#include "string.h"

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

__attribute__((interrupt)) void int_handler_keyboard(struct interrupt_frame *frame)
{
    uint8_t scancode = inb(0x60); // ps/2 keyboard port
    kbd_handle_input(scancode);
    pic_eoi(IRQ_KBD_PS2);
}

__attribute__((interrupt)) void int_handler_mouse(struct interrupt_frame *frame)
{
    uint8_t data = ps2_mouse_read();
    ps2_mouse_process_input(data);
    pic_eoi(IRQ_MOUSE_PS2);
}

__attribute__((interrupt)) void int_handler_pit(struct interrupt_frame *frame)
{
    pit_tick();
    pic_eoi(IRQ_SYSTEM_TIMER);
}