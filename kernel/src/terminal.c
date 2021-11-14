#include "terminal.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "types.h"
#include "math.h"
#include "terminal.h"

#define FONT_HEIGHT 16
#define FONT_WIDTH 8

framebuffer_t *_framebuffer;
psf1_font_t *_font;
point_t _cursor_pos;
unsigned int _fgcolor;
unsigned int _bgcolor;
unsigned int _bytes_per_pixel;
bool _enabled = true;
uint32_t _overlay_buffer_pre[16 * 16];
uint32_t _overlay_buffer_post[16 * 16];
static void __put_char(const char chr, point_t *pos);
static void __clear_char(point_t *pos);
static void __scroll();

void terminal_init(framebuffer_t *framebuffer, psf1_font_t *font)
{
    _framebuffer = framebuffer;
    _font = font;
    _bytes_per_pixel = 4;
    _fgcolor = 0xFFFFFFFF;
    _bgcolor = 0x00000000;
    terminal_clear();
}

void terminal_put_pixel(unsigned int x, unsigned int y, unsigned int color)
{
    if (!_enabled) return;
    unsigned int *pixelPtr = (unsigned int *)_framebuffer->base_address;
    *(unsigned int*)(pixelPtr + x + (y * _framebuffer->pixels_per_scan_line)) = color;
}

unsigned int terminal_get_pixel(unsigned int x , unsigned int y)
{
    unsigned int *pixelPtr = (unsigned int *)_framebuffer->base_address;
    return *(unsigned int*)(pixelPtr + x + (y * _framebuffer->pixels_per_scan_line));
}

void terminal_put_char(const char chr)
{
    if (!_enabled) return;
    if (chr == '\n') {
        terminal_newline();
        return;
    }

   __put_char(chr, &_cursor_pos);

    _cursor_pos.x += FONT_WIDTH;
    if (_cursor_pos.x >= _framebuffer->horizontal_resolution) {
        terminal_newline();
    }
}

void terminal_put_string(const char *str, unsigned int length)
{
    for(int i = 0; i < length; i++)
        terminal_put_char(*(str+i));
}

void terminal_print(const char *str)
{
    for(const char *chr = str; *chr != 0; chr++)
    {
        terminal_put_char(*chr);
    }
}

void terminal_nprint(size_t count, ...)
{
    va_list args;
    va_start(args, count);
    for(int i = 0; i < count; i++) {
        terminal_print(va_arg(args, char*));
    }
    va_end(args);
}

void terminal_println(const char *str)
{
    terminal_print(str);
    terminal_put_char('\n');

}

void terminal_nprintln(size_t count, ...)
{
    va_list args;
    va_start(args, count);
    for(int i = 0; i < count; i++) {
        terminal_print(va_arg(args, char*));
    }
    va_end(args);
    terminal_put_char('\n');
}

void terminal_move_cursor(unsigned int x, unsigned int y)
{
    _cursor_pos.x = x <= _framebuffer->horizontal_resolution - FONT_WIDTH ? x : _framebuffer->horizontal_resolution - FONT_WIDTH;
    _cursor_pos.y = y <= _framebuffer->vertical_resolution - FONT_HEIGHT ? y : _framebuffer->vertical_resolution - FONT_HEIGHT;
}

void terminal_set_fgcolor(unsigned int color)
{
    _fgcolor = color;
}

void terminal_set_bgcolor(unsigned int color)
{
    _bgcolor = color;
}

void terminal_set_color(unsigned int fgcolor, unsigned int bgcolor)
{
    _fgcolor = fgcolor;
    _bgcolor = bgcolor;
}

void terminal_clear()
{
    for(int y=0; y<=_framebuffer->vertical_resolution; y++) {
        for(int x=0; x<=_framebuffer->horizontal_resolution; x++) {
            terminal_put_pixel(x, y, _bgcolor);
        }
    }

    terminal_move_cursor(0, 0);
}

void terminal_disable()
{
    _enabled = false;
}

void terminal_enable()
{
    _enabled = true;
}

void terminal_newline()
{
    _cursor_pos.x = 0;
    _cursor_pos.y += FONT_HEIGHT;

    if (_cursor_pos.y >= _framebuffer->vertical_resolution) {
        _cursor_pos.y =_framebuffer->vertical_resolution - FONT_HEIGHT;
        __scroll();
    }
}

void terminal_backspace()
{
    if (_cursor_pos.x < FONT_WIDTH && _cursor_pos.y < FONT_HEIGHT) return;

    if (_cursor_pos.x < FONT_WIDTH) {
        _cursor_pos.x = _framebuffer->horizontal_resolution - FONT_WIDTH;
        _cursor_pos.y -= FONT_HEIGHT;
    } else {
        _cursor_pos.x -= FONT_WIDTH;
    }

    __clear_char(&_cursor_pos);
}

void terminal_draw_overlay(uint8_t *data, point_t *pos, unsigned int color)
{
    int x_pad = _framebuffer->horizontal_resolution - pos->x;
    int y_pad = _framebuffer->vertical_resolution - pos->y;
    int width = x_pad > 16 ? 16 : x_pad;
    int height = y_pad > 16 ? 16 : y_pad;

    int max_x = _framebuffer->horizontal_resolution - 1;
    int max_y = _framebuffer->vertical_resolution - 1;

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
                _overlay_buffer_pre[j + i * 16] = terminal_get_pixel(x + j, y + i);
                terminal_put_pixel(x + j, y + i, color);
                _overlay_buffer_post[j + i * 16] = color;
            }
        }
    }
}

void terminal_clear_overlay(uint8_t *data, point_t *pos)
{
    int x_pad = _framebuffer->horizontal_resolution - pos->x;
    int y_pad = _framebuffer->vertical_resolution - pos->y;
    int width = x_pad > 16 ? 16 : x_pad;
    int height = y_pad > 16 ? 16 : y_pad;

    int max_x = _framebuffer->horizontal_resolution - 1;
    int max_y = _framebuffer->vertical_resolution - 1;

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
                int current_color = terminal_get_pixel(x + j, y + i);
                if (current_color == _overlay_buffer_post[j + i * 16]) {
                    int color = _overlay_buffer_pre[j + i * 16];
                    terminal_put_pixel(x + j, y + i, color);
                }
            }
        }
    }
}

unsigned int terminal_vertical_resolution()
{
    return _framebuffer->vertical_resolution;
}

unsigned int terminal_horizontal_resolution()
{
    return _framebuffer->horizontal_resolution;
}

static void __put_char(const char chr, point_t *pos)
{
    char *fontPtr = (char *)_font->glyph_buffer + (chr * _font->header->char_size); // index of glyph within glyph array
    for (unsigned long y = pos->y; y < pos->y + FONT_HEIGHT; y++) {
        for (unsigned long x = pos->x; x < pos->x + FONT_WIDTH; x++) {
            if ((*fontPtr & (0b10000000 >> (x - pos->x))) > 0) {
                terminal_put_pixel(x, y, _fgcolor);
            }
        }
        fontPtr++;
    }
}

static void __clear_char(point_t *pos)
{
    for (unsigned long y = pos->y; y < pos->y + FONT_HEIGHT; y++) {
        for (unsigned long x = pos->x; x < pos->x + FONT_WIDTH; x++) {
            terminal_put_pixel(x, y, _bgcolor);
        }
    }
}

static void __scroll()
{
    for(int y=FONT_HEIGHT; y<=_framebuffer->vertical_resolution; y++) {
        for(int x=0; x<=_framebuffer->horizontal_resolution; x++) {
            unsigned int p = terminal_get_pixel(x, y);
            terminal_put_pixel(x, y-FONT_HEIGHT, p);
        }
    }
    for(int y=_framebuffer->vertical_resolution - FONT_HEIGHT; y <= _framebuffer->vertical_resolution; y++) {
        for(int x=0; x<=_framebuffer->horizontal_resolution; x++) {
            terminal_put_pixel(x, y, 0x00000000);
        }
    }
}