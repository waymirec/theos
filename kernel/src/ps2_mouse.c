#include "ps2_mouse.h"

#include "ps2.h"
#include "io.h"

#include "terminal.h"
#include "string.h"

#define WAIT_TIMEOUT 100000

static void __wait_for_write(void);
static void __wait_for_read(void);

void ps2_mouse_init(void)
{
    uint8_t val;
    char buff[128];

    // enable second PS/2 port (mouse)
    outb(PS2_COMMAND_PORT, PS2_CMD_ENABLE_AUX);

    __wait_for_write();
    outb(PS2_COMMAND_PORT, PS2_CMD_READ_CFG);
    __wait_for_read();

    // enable interrupts for second ps2/ port (mouse)
    uint8_t status = (inb(PS2_DATA_PORT) | 0b10);

    __wait_for_write();   
    outb(PS2_COMMAND_PORT, PS2_CMD_WRITE_CFG);
    __wait_for_write();
    outb(PS2_DATA_PORT, status);

    val = ps2_mouse_write(PS2_CMD_SET_DEFAULTS);
    terminal_nprintln(2, "val: ", uint8_to_hex(val, buff));

    val = ps2_mouse_write(PS2_CMD_ENABLE_DATA_REPORTING);
    terminal_nprintln(2, "val: ", uint8_to_hex(val, buff));
}

uint8_t ps2_mouse_write(uint8_t value)
{
    __wait_for_write();
    outb(PS2_COMMAND_PORT, PS2_CMD_WRITE_AUX);

    __wait_for_write();
    outb(PS2_DATA_PORT, value);

    __wait_for_read();
    return inb(PS2_DATA_PORT);
}

uint8_t ps2_mouse_read(void)
{
    __wait_for_read();
    return inb(PS2_DATA_PORT);
}

//mousewait
static void __wait_for_write(void)
{
    uint64_t timeout = WAIT_TIMEOUT;
    while(timeout--) 
        if ((inb(PS2_STATUS_REG) & 0b10) == 0) return;
}

//mousewaitinput
static void __wait_for_read(void)
{
    uint64_t timeout = WAIT_TIMEOUT;
    while(timeout--) 
        if (inb(PS2_STATUS_REG) & 0b1) return;
}
