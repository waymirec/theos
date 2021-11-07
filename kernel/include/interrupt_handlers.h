#pragma once

struct interrupt_frame;

__attribute__((interrupt)) void int_handler_pagefault(struct interrupt_frame *frame);
__attribute__((interrupt)) void int_handler_double_fault(struct interrupt_frame *frame);
__attribute__((interrupt)) void int_handler_general_protection(struct interrupt_frame *frame);
__attribute__((interrupt)) void int_handler_keyboard(struct interrupt_frame *frame);
__attribute__((interrupt)) void int_handler_mouse(struct interrupt_frame *frame);