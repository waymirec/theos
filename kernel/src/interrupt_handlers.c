#include "interrupt_handlers.h"
#include "types.h"
#include "panic.h"
#include "io.h"
#include "8259_pic.h"
#include "irq.h"

#include "ps2_keyboard.h" // todo: refactor this
#include "ps2_mouse.h"
#include "terminal.h"

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

__attribute__((interrupt)) void int_handler_keyboard(struct interrupt_frame *frame)
{
    uint8_t scancode = inb(0x60); // ps/2 keyboard port
    pic_eoi(IRQ_KBD_PS2);
    kbd_handle_input(scancode);
}

__attribute__((interrupt)) void int_handler_mouse(struct interrupt_frame *frame)
{
    uint8_t data = inb(0x60);
    terminal_put_char('#');
    //uint8_t data = ps2_mouse_read();
    pic_eoi(IRQ_MOUSE_PS2);
}