#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "display.h"
#include "font.h"
#include "math.h"

typedef void (*tty_puts_fun)(const char *str);

typedef struct tty_t tty_t;

struct tty_t {
    framebuffer_t *framebuffer;
    unsigned int bytes_per_pixel;

    psf1_font_t *font;
    unsigned int font_width;
    unsigned int font_height;

    point_t cursor_pos;
    unsigned int fgcolor;
    unsigned int bgcolor;

    uint32_t overlay_buffer_pre[16 * 16];
    uint32_t overlay_buffer_post[16 * 16];
    bool enabled;
};

void tty_init(tty_t *tty, framebuffer_t *framebuffer, psf1_font_t *font);
void tty_putc(tty_t *tty, const char chr);
void tty_puts(tty_t *tty, const char *str);
void tty_move_cursor(tty_t *tty, unsigned int x, unsigned int y);
void tty_clear(tty_t *tty);
void tty_newline(tty_t *tty);
void tty_backspace(tty_t *tty);
void tty_draw_overlay(tty_t *tty, uint8_t *data, point_t *pos, unsigned int color);
void tty_clear_overlay(tty_t *tty, uint8_t *data, point_t *pos);
unsigned int tty_height(tty_t *tty);
unsigned int tty_width(tty_t *tty);

