#include "tty.h"

#include <string.h>

void tty_init(tty_t *tty, framebuffer_t *framebuffer, psf1_font_t *font)
{
    tty->framebuffer = framebuffer;
    tty->bytes_per_pixel = 4;
    tty->font = font;
    tty->font_width = 8;
    tty->font_height = font->header->char_size;
    tty->fgcolor = 0xFFFFFFFF;
    tty->bgcolor = 0x00000000;
    tty->enabled = true;

    tty->cursor_pos.x = 0;
    tty->cursor_pos.y = 0;

    memzero(tty->overlay_buffer_pre, (sizeof(tty->overlay_buffer_pre) / sizeof(tty->overlay_buffer_pre[0])));
    memzero(tty->overlay_buffer_post, (sizeof(tty->overlay_buffer_post) / sizeof(tty->overlay_buffer_post[0])));    
    tty_clear(tty);
}

static void __put_pixel(tty_t *tty, unsigned int x, unsigned int y, unsigned int color)
{
    if (!tty->enabled) return;
    unsigned int *pixelPtr = (unsigned int *)tty->framebuffer->base_address;
    *(unsigned int*)(pixelPtr + x + (y * tty->framebuffer->pixels_per_scan_line)) = color;
}

static unsigned int __get_pixel(tty_t *tty, unsigned int x , unsigned int y)
{
    unsigned int *pixelPtr = (unsigned int *)tty->framebuffer->base_address;
    return *(unsigned int*)(pixelPtr + x + (y * tty->framebuffer->pixels_per_scan_line));
}

static void __put_char(tty_t *tty, const char chr, point_t *pos)
{
    char *fontPtr = (char *)tty->font->glyph_buffer + (chr * tty->font->header->char_size); // index of glyph within glyph array
    for (unsigned long y = pos->y; y < pos->y + tty->font_height; y++) {
        for (unsigned long x = pos->x; x < pos->x + tty->font_width; x++) {
            if ((*fontPtr & (0b10000000 >> (x - pos->x))) > 0) {
                __put_pixel(tty, x, y, tty->fgcolor);
            }
        }
        fontPtr++;
    }
}

static void __clear_char(tty_t *tty, point_t *pos)
{
    for (unsigned long y = pos->y; y < pos->y + tty->font_height; y++) {
        for (unsigned long x = pos->x; x < pos->x + tty->font_width; x++) {
            __put_pixel(tty, x, y, tty->bgcolor);
        }
    }
}

static void __scroll(tty_t *tty)
{
    for(int y=tty->font_height; y<=tty->framebuffer->vertical_resolution; y++) {
        for(int x=0; x<=tty->framebuffer->horizontal_resolution; x++) {
            unsigned int p = __get_pixel(tty, x, y);
            __put_pixel(tty, x, y-tty->font_height, p);
        }
    }
    for(int y=tty->framebuffer->vertical_resolution - tty->font_height; y <= tty->framebuffer->vertical_resolution; y++) {
        for(int x=0; x<=tty->framebuffer->horizontal_resolution; x++) {
            __put_pixel(tty, x, y, 0x00000000);
        }
    }
}

void tty_putc(tty_t *tty, const char chr)
{
    if (!tty->enabled) return;
    if (chr == '\n') {
        tty_newline(tty);
        return;
    }

   __put_char(tty, chr, &tty->cursor_pos);

    tty->cursor_pos.x += tty->font_width;
    if (tty->cursor_pos.x >= tty->framebuffer->horizontal_resolution) {
        tty_newline(tty);
    }
}

void tty_puts(tty_t *tty, const char *str)
{
    for(const char *chr = str; *chr != 0; chr++)
    {
        tty_putc(tty, *chr);
    }
}

void tty_move_cursor(tty_t *tty, unsigned int x, unsigned int y)
{
    tty->cursor_pos.x = x <= tty->framebuffer->horizontal_resolution - tty->font_width ? x : tty->framebuffer->horizontal_resolution - tty->font_width;
    tty->cursor_pos.y = y <= tty->framebuffer->vertical_resolution - tty->font_height ? y : tty->framebuffer->vertical_resolution - tty->font_height;
}

void tty_clear(tty_t *tty)
{
    for(int y = 0; y <= tty->framebuffer->vertical_resolution; y++) {
        for(int x = 0; x <= tty->framebuffer->horizontal_resolution; x++) {
            __put_pixel(tty, x, y, tty->bgcolor);
        }
    }

    tty_move_cursor(tty, 0, 0);
}

void tty_newline(tty_t *tty)
{
    tty->cursor_pos.x = 0;
    tty->cursor_pos.y += tty->font_height;

    if (tty->cursor_pos.y >= tty->framebuffer->vertical_resolution) {
        tty->cursor_pos.y = tty->framebuffer->vertical_resolution - tty->font_height;
        __scroll(tty);
    }
}

void tty_backspace(tty_t *tty)
{
    if (tty->cursor_pos.x < tty->font_width && tty->cursor_pos.y < tty->font_height) return;

    if (tty->cursor_pos.x < tty->font_width) {
        tty->cursor_pos.x = tty->framebuffer->horizontal_resolution - tty->font_width;
        tty->cursor_pos.y -= tty->font_height;
    } else {
        tty->cursor_pos.x -= tty->font_width;
    }

    __clear_char(tty, &tty->cursor_pos);
}

void tty_draw_overlay(tty_t *tty, uint8_t *data, point_t *pos, unsigned int color)
{
    int x_pad = tty->framebuffer->horizontal_resolution - pos->x;
    int y_pad = tty->framebuffer->vertical_resolution - pos->y;
    int width = x_pad > 16 ? 16 : x_pad;
    int height = y_pad > 16 ? 16 : y_pad;

    int max_x = tty->framebuffer->horizontal_resolution - 1;
    int max_y = tty->framebuffer->vertical_resolution - 1;

    int x = pos->x;
    if (x > max_x) x = max_x;
    if (x < 0) x = 0;

    int y = pos->y;
    if (y > max_y) y = max_y;
    if (y < 0) y = 0;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int byte = (i * 16 + j) / 8;
            if ((data[byte] & (0b10000000 >> (j % 8)))) {
                tty->overlay_buffer_pre[j + i * 16] = __get_pixel(tty, x + j, y + i);
                __put_pixel(tty, x + j, y + i, color);
                tty->overlay_buffer_post[j + i * 16] = color;
            }
        }
    }
}

void tty_clear_overlay(tty_t *tty, uint8_t *data, point_t *pos)
{
    int x_pad = tty->framebuffer->horizontal_resolution - pos->x;
    int y_pad = tty->framebuffer->vertical_resolution - pos->y;
    int width = x_pad > 16 ? 16 : x_pad;
    int height = y_pad > 16 ? 16 : y_pad;

    int max_x = tty->framebuffer->horizontal_resolution - 1;
    int max_y = tty->framebuffer->vertical_resolution - 1;

    int x = pos->x;
    if (x > max_x) x = max_x;
    if (x < 0) x = 0;

    int y = pos->y;
    if (y > max_y) y = max_y;
    if (y < 0) y = 0;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int byte = (i * 16 + j) / 8;
            if ((data[byte] & (0b10000000 >> (j % 8)))) {
                int current_color = __get_pixel(tty, x + j, y + i);
                if (current_color == tty->overlay_buffer_post[j + i * 16]) {
                    int color = tty->overlay_buffer_pre[j + i * 16];
                    __put_pixel(tty, x + j, y + i, color);
                }
            }
        }
    }
}

unsigned int tty_height(tty_t *tty)
{
    return tty->framebuffer->vertical_resolution / tty->font_height;
}

unsigned int tty_width(tty_t *tty)
{
    return tty->framebuffer->horizontal_resolution / tty->font_width;
}
