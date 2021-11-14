#pragma once

#include "types.h"
#include "font.h"
#include "boot.h"
#include "math.h"

#include <stdint.h>
#include <stddef.h>

void terminal_init(framebuffer_t *framebuffer, psf1_font_t *font);
void terminal_put_pixel(unsigned int x, unsigned int y, unsigned int color);
void terminal_put_char(const char chr);
void terminal_put_string(const char *str, unsigned int length);
void terminal_print(const char *str);
void terminal_nprint(size_t count, ...);
void terminal_println(const char *str);
void terminal_nprintln(size_t count, ...);
void terminal_set_fgcolor(unsigned int color);
void terminal_set_bgcolor(unsigned int color);
void terminal_set_color(unsigned int fgcolor, unsigned int bgcolor);
void terminal_move_cursor(unsigned int x, unsigned int y);
void terminal_clear();
void terminal_enable();
void terminal_disable();
void terminal_newline();
void terminal_backspace();
void terminal_draw_overlay(uint8_t *data, point_t *pos, unsigned int color);
void terminal_clear_overlay(uint8_t *data, point_t *pos);

unsigned int terminal_vertical_resolution();
unsigned int terminal_horizontal_resolution();
