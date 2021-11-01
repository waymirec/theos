#include "terminal.h"
#include "types.h"
#include "math.h"
#include "terminal.h"

#include <stdarg.h>

#define FONT_HEIGHT 16
#define FONT_WIDTH 8

framebuffer_t *_framebuffer;
psf1_font_t *_font;
point_t _cursor_pos;
unsigned int _color;
unsigned int _bytes_per_pixel;
bool _enabled = true;

void newline();
void scroll();

void terminal_init(framebuffer_t *framebuffer, psf1_font_t *font)
{
    _framebuffer = framebuffer;
    _font = font;
    _bytes_per_pixel = 4;
    _color = 0xFFFFFFFF;
    terminal_clear(0x00000000);
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
        newline();
        return;
    }

    char *fontPtr = (char *)_font->glyph_buffer + (chr * _font->header->char_size); // index of glyph within glyph array
    for (unsigned long y = _cursor_pos.y; y < _cursor_pos.y + FONT_HEIGHT; y++) {
        for (unsigned long x = _cursor_pos.x; x < _cursor_pos.x + FONT_WIDTH; x++) {
            if ((*fontPtr & (0b10000000 >> (x - _cursor_pos.x))) > 0) {
                terminal_put_pixel(x, y, _color);
            }
        }
        fontPtr++;
    }

    _cursor_pos.x += FONT_WIDTH;
    if (_cursor_pos.x >= _framebuffer->horizontal_resolution) {
        newline();
    }
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
    _cursor_pos.x = x <= _framebuffer->horizontal_resolution ? x : _framebuffer->horizontal_resolution;
    _cursor_pos.y = y <= _framebuffer->vertical_resolution ? y : _framebuffer->vertical_resolution;
}

void terminal_set_color(unsigned int color)
{
    _color = color;
}

void terminal_clear(unsigned int color)
{
    for(int y=0; y<=_framebuffer->vertical_resolution; y++) {
        for(int x=0; x<=_framebuffer->horizontal_resolution; x++) {
            terminal_put_pixel(x, y, color);
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

void newline()
{
    _cursor_pos.x = 0;
    _cursor_pos.y += FONT_HEIGHT;

    if (_cursor_pos.y >= _framebuffer->vertical_resolution) {
        _cursor_pos.y =_framebuffer->vertical_resolution - FONT_HEIGHT;
        scroll();
    }
}

void scroll()
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