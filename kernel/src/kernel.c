#include "types.h"
#include "terminal.h"
#include "font.h"
#include "string.h"
#include "memory.h"
#include "paging.h"
#include "pageframe_allocator.h"
#include "pagetable_manager.h"
#include "boot.h"
#include "gdt.h"
#include "idt.h"
#include "8259_pic.h"
#include "io.h"
#include "ps2_mouse.h"

#include <stdint.h>

pml4_t *g_pml4;

void initialize_kernel(boot_info_t *boot_info);
void setup_terminal(boot_info_t *boot_info);
void setup_paging(boot_info_t *boot_info);
void setup_interrupts();
void display_banner(boot_info_t *boot_info);
void loop();

char terminal_buffer[128];

void _start(boot_info_t *boot_info)
{
    initialize_kernel(boot_info);
    display_banner(boot_info);

    loop();
}

void initialize_kernel(boot_info_t *boot_info)
{
    setup_terminal(boot_info);
    setup_paging(boot_info);
    gdt_init();
    setup_interrupts();
    ps2_mouse_init();

    outb(PIC1_DATA, 0b11111001);  // unmask keyboard (IRQ1) and cascade (IRQ2)
    outb(PIC2_DATA, 0b11101111);  // unmask PS/2 aux (IRQ12)
    asm("sti");
}

void setup_terminal(boot_info_t *boot_info)
{
    terminal_init(boot_info->framebuffer, boot_info->font);
}

void setup_paging(boot_info_t *boot_info)
{
    memory_info_t *memory_info = boot_info->memory_info;
    pageframe_allocator_init(memory_info);
    g_pml4 = (pml4_t *)pageframe_request();
    pagetable_init(g_pml4, boot_info);
}

void setup_interrupts()
{
    idt_init();
    pic_remap(0x20, 0x28);
}

void display_banner(boot_info_t *boot_info)
{
    terminal_println("Welcome to theOS!");
    terminal_nprintln(3, "Memory Free: ", int_to_string(pageframe_memory_free() / 1024, terminal_buffer), "KB"); 
    terminal_nprintln(3, "Memory Used: ", int_to_string(pageframe_memory_used() / 1024, terminal_buffer), "KB"); 
    terminal_nprintln(3, "Memory Reserved: ", int_to_string(pageframe_memory_reserved() / 1024, terminal_buffer), "KB"); 
}

void loop()
{
    while(true);
}