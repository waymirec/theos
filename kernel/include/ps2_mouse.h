#pragma once

#include <stdint.h>

void ps2_mouse_init(void);
void ps2_mouse_process_input(uint8_t data);
void ps2_mouse_handle_input();
uint8_t ps2_mouse_write(uint8_t value);
uint8_t ps2_mouse_read(void);