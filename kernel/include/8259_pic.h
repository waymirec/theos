#pragma once

#include <stdint.h>

/* Master PIC - IRQ: 00-07, Vector Offset: 0x08, Interrupt Numbers: 0x08 - 0x0F */
/*  Slave PIC - IRQ: 08-15, Vector Offset: 0x70, Interrupt Numbers: 0x70 - 0x77 */

#define PIC1                0x20		/* IO base address for master PIC */
#define PIC2                0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND        PIC1
#define PIC1_DATA           (PIC1+1)
#define PIC2_COMMAND        PIC2
#define PIC2_DATA           (PIC2+1)

#define PIC_EOI             0x20        /* End-of-Interrupt command code */
#define PIC_READ_IRR        0x0A        /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR        0x0B        /* OCW3 irq service next CMD read */
 
#define ICW1_ICW4           0X01        /* ICW4 (Not) needed */
#define ICW1_SINGLE         0x02        /* Single (cascade) mode */
#define ICW1_INTERVAL4      0x04        /* Call address interval 4 (8) */
#define ICW1_LEVEL          0x08        /* Level triggered (edge) mode */
#define ICW1_INIT           0x10        /* Initialization - required! */

#define ICW4_8086           0x01        /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO           0x02        /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE      0x08        /* Buffered mode/slave */
#define ICW4_BUF_MASTER     0x0C        /* Buffered mode/master */
#define ICW4_SFNM           0x10        /* Special fully nested (not) */


void pic_eoi(unsigned char irq);
void pic_remap(int offset1, int offset2);
void pic_set_irq_mask(unsigned char irq);
void pic_clear_irq_mask(unsigned char irq);
uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);