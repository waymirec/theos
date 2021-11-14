#include "ps2_mouse.h"

#include "ps2.h"
#include "io.h"
#include "pageframe_allocator.h"
#include "math.h"

#include "terminal.h"
#include "string.h"

#define WAIT_TIMEOUT 100000
#define MAX_PACKETS 100
#define PACKET_SIZE 3

static uint8_t _bytes[PACKET_SIZE];
static uint8_t _byte_count = 0;
static bool _ready = false;
static point_t _mouse_pos = { 0 };
static point_t _mouse_pos_prev = { 0 };
static bool _mouse_drawn = false;

static unsigned int _horiz_res;
static unsigned int _vert_res;

static uint8_t _mouse_cursor_overlay[] = {
    0b11111111, 0b11100000, 
    0b11111111, 0b10000000, 
    0b11111110, 0b00000000, 
    0b11111100, 0b00000000, 
    0b11111000, 0b00000000, 
    0b11110000, 0b00000000, 
    0b11100000, 0b00000000, 
    0b11000000, 0b00000000, 
    0b11000000, 0b00000000, 
    0b10000000, 0b00000000, 
    0b10000000, 0b00000000, 
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 
};

static void __wait_for_write(void);
static void __wait_for_read(void);
static void __move_mouse(mouse_data);

void ps2_mouse_init(void)
{
    _vert_res = terminal_vertical_resolution();
    _horiz_res = terminal_horizontal_resolution();

    uint8_t val;

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
    //todo: assert val == 0xFA

    val = ps2_mouse_write(PS2_CMD_ENABLE_DATA_REPORTING);
    //todo: assert val == 0xFA
}

void ps2_mouse_process_input(uint8_t data)
{
    if (_ready) return;
    if (_byte_count == 0 && (data & 0b00001000 == 0)) return;

    _bytes[_byte_count++] = data;

    if (_byte_count == PACKET_SIZE)
        _ready = true;

}

void ps2_mouse_handle_input()
{
    if (!_ready) return;

    mouse_data data = { .bytes[0] = _bytes[0], .bytes[1] = _bytes[1], .bytes[2] = _bytes[2] };
    
    if (_mouse_drawn)
        terminal_clear_overlay(_mouse_cursor_overlay, &_mouse_pos);

    __move_mouse(data);
    terminal_draw_overlay(_mouse_cursor_overlay, &_mouse_pos, 0xFF00FFFF);
    _mouse_drawn = true;
    
    _byte_count = 0;
    _ready = false;
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

static void __wait_for_write(void)
{
    uint64_t timeout = WAIT_TIMEOUT;
    while(timeout--) 
        if ((inb(PS2_STATUS_REG) & 0b10) == 0) return;
}

static void __wait_for_read(void)
{
    uint64_t timeout = WAIT_TIMEOUT;
    while(timeout--) 
        if (inb(PS2_STATUS_REG) & 0b1) return;
}

static void __move_mouse(mouse_data data)
{
    _mouse_pos_prev = _mouse_pos;

    if (!data.x_negative) {
        _mouse_pos.x += data.x_data;

        if (data.x_overflow) 
            _mouse_pos.x += 255;
    } else {
        _mouse_pos.x -= (256 - data.x_data);
        if (data.x_overflow) 
            _mouse_pos.x -= 255;
    }

    if (!data.y_negative) {
        _mouse_pos.y -= data.y_data;
        if (data.y_overflow) 
            _mouse_pos.y -= 255;
    } else {
        _mouse_pos.y += (256 - data.y_data);
        if (data.y_overflow) 
            _mouse_pos.y += 255;
    }

    if (_mouse_pos.x < 0) _mouse_pos.x = 0;
    if (_mouse_pos.x > _horiz_res - 1) _mouse_pos.x = _horiz_res - 1;

    if (_mouse_pos.y < 0) _mouse_pos.y = 0;
    if (_mouse_pos.y > _vert_res - 1) _mouse_pos.y = _vert_res - 1;
}
