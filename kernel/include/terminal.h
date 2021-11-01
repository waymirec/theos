#pragma once

#include "types.h"
#include "font.h"
#include "boot.h"

#include <stdint.h>
#include <stddef.h>

void terminal_init(framebuffer_t *framebuffer, psf1_font_t *font);
void terminal_put_pixel(unsigned int x, unsigned int y, unsigned int color);
void terminal_put_char(const char chr);
void terminal_print(const char *str);
void terminal_nprint(size_t count, ...);
void terminal_println(const char *str);
void terminal_nprintln(size_t count, ...);
void terminal_set_color(unsigned int color);
void terminal_move_cursor(unsigned int x, unsigned int y);
void terminal_clear(unsigned int color);
void terminal_enable();
void terminal_disable();
void terminal_newline();