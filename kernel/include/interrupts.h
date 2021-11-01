#pragma once

struct interrupt_frame;

__attribute__((interrupt)) void pagefault_handler(struct interrupt_frame *frame);