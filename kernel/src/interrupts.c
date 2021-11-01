#include "interrupts.h"
#include "types.h"
#include "terminal.h"

__attribute__((interrupt)) void pagefault_handler(struct interrupt_frame *frame)
{
    terminal_println("PAGE FAULT!");
    while(true);
}