#include "8259_pic.h"
#include "io.h"

static uint16_t __pic_get_irq_reg(int ocw3);

// This is issued to the PIC chips at the end of an IRQ-based interrupt routine.
// If the IRQ came from the Master PIC, it is sufficient to issue this command 
// only to the Master PIC; however if the IRQ came from the Slave PIC, it is 
// necessary to issue the command to both PIC chips.
void pic_eoi(unsigned char irq)
{
    if (irq >= 8) 
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

// When you enter protected mode (or even before), the first command needed to be given
// the two PICs is the initialize command (code 0x11). This command makes the PIC wait 
// for 3 extra "initialization words" on the data port. These bytes give the PIC:
// - its vector offset (ICW2)
// - tell it how it is wired to master/slaves (ICW3)
// - gives additional information about the environment (ICW4)
// arguments:
//          offset1 - vector offset for Master PIC
//                    vectors on the master become offset1..offset1+7
//          offset2 - same for the Slave PIC: offset2..offset2+7
void pic_remap(int offset1, int offset2)
{
    unsigned char masks;
    
    masks = inb(PIC1_DATA);                             // save masks
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);          // starts the initialization sequence (in cascade mode)
    io_wait();
    outb(PIC1_DATA, offset1);                           // ICW2: Master PIC vector offset
    io_wait();
    outb(PIC1_DATA, 4);                                 // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC1_DATA, masks);                             // restore saved masks

    masks = inb(PIC2_DATA);                             // save masks
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_DATA, offset2);                           // ICW2: Slave PIC vector offset
    io_wait();
    outb(PIC2_DATA, 2);                                 // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, masks);                             // restore saved masks
}

// The PIC has an internal register called the IMR, or the Interrupt Mask Register. 
// It is 8 bits wide. This register is a bitmap of the request lines going into the 
// PIC. When a bit is set, the PIC ignores the request and continues normal operation. 
// Note that setting the mask on a higher request line will not affect a lower line. 
// Masking IRQ2 will cause the Slave PIC to stop raising IRQs.
void pic_set_irq_mask(unsigned char irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

void pic_clear_irq_mask(unsigned char irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

// Returns the combined value of the cascaded PICs irq request register
uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

// Returns the combined value of the cascaded PICs in-service register
uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}

static uint16_t __pic_get_irq_reg(int ocw3)
{
    // OCW3 to PIC CMD to get the register values. PIC2 is chained,
    // and represents IRQs 8-15. PIC1 is IRQs 0-7, with 2 being the chain
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8 | inb(PIC1_COMMAND));
}