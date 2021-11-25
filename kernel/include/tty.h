#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "display.h"
#include "font.h"
#include "math.h"

typedef struct {
    void *framebuffer;
    psf1_font_t *font;
    unsigned int _font_height;
    unsigned int _font_width;
    point_t cursor_pos;
    unsigned int fgcolor;
    unsigned int bgcolor;
    unsigned int bytes_per_pixel;
    bool _enabled;
    uint32_t overlay_buffer_pre[16 * 16];
    uint32_t overlay_buffer_post[16 * 16];
} tty_t;

tty_t* tty_init(framebuffer_t *framebuffer, psf1_font_t *font);
