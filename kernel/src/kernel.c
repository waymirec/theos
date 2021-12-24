#include "kernel.h"

#include <stdint.h>

#include "types.h"
#include "globals.h"
#include "tty.h"
#include "font.h"
#include "stdio.h"
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
#include "acpi.h"
#include "pci.h"
#include "heap.h"
#include "pit.h"

void initialize_kernel(boot_info_t *boot_info);
void setup_terminal(boot_info_t *boot_info);
void setup_paging(boot_info_t *boot_info);
void setup_interrupts(void);
void setup_acpi(boot_info_t *boot_info);
void display_banner(boot_info_t *boot_info);
void loop();

pml4_t *g_pml4 = NULL;

tty_t tty;
tty_t *g_tty = &tty;

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
    heap_init((void *)0x0000100000000000, 0x10);
    gdt_init();
    setup_interrupts();
    ps2_mouse_init();
    setup_acpi(boot_info);
    pit_init(100); // 100hz == 100 ticks / second

    outb(PIC1_DATA, 0b11111000);  // unmask PIT (IRQ0), keyboard (IRQ1) and cascade (IRQ2)
    outb(PIC2_DATA, 0b11101111);  // unmask PS/2 aux (IRQ12)
    asm("sti");
}

void setup_terminal(boot_info_t *boot_info)
{
    tty_init(g_tty, boot_info->framebuffer, boot_info->font);
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

void setup_acpi(boot_info_t *boot_info)
{
    acpi_sdt_header_t *xsdt = (acpi_sdt_header_t *)(boot_info->rootSystemDescriptionPointer->xsdt_address);
    acpi_mcfg_header_t *mcfg = (acpi_mcfg_header_t *)acpi_find_table(xsdt, (char *)"MCFG");

    pci_enumerate(mcfg);
}

void display_banner(boot_info_t *boot_info)
{
    printf("Welcome to theOS!!\n");
    printf("Memory Free: %u\n", (pageframe_memory_free() / 1024));
    printf("Memory Used: %u\n", (pageframe_memory_used() / 1024));
    printf("Memory Rsvd: %u\n", (pageframe_memory_used() / 1024));
}

void loop()
{
    while(true) {
        ps2_mouse_handle_input();
        asm("hlt");
    }
}