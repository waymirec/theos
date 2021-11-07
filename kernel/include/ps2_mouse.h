#pragma once

#include <stdint.h>

void ps2_mouse_init(void);
uint8_t ps2_mouse_write(uint8_t value);
uint8_t ps2_mouse_read(void);