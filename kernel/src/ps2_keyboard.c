#include "ps2_keyboard.h"

#include <stdbool.h>

#include "globals.h"
#include "tty.h"
#include "string.h"

#define MAX_PRINTABLE_SCANCODE 57
#define SPACE_PRESSED 0x39
#define LSHIFT_PRESSED 0x2A
#define LSHIFT_RELEASED 0xAA
#define RSHIFT_PRESSED 0x36
#define RSHIFT_RELEASED 0xB6
#define CAPSLOCK_PRESSED 0x3A
#define CAPSLOCK_RELEASED 0xBA
#define BSPACE_PRESSED 0x0E
#define BSPACE_RELEASED 0x8E

static bool _lshift_pressed = false;
static bool _rshift_pressed = false;
static bool _bspace_pressed = false;
static bool _capslock_pressed = false;

static const char _ascii_table[] = {
         0 ,  0 , '1', '2',
        '3', '4', '5', '6',
        '7', '8', '9', '0',
        '-', '=',  0 ,  0 ,
        'q', 'w', 'e', 'r',
        't', 'y', 'u', 'i',
        'o', 'p', '[', ']',
        '\n', 0 , 'a', 's',
        'd', 'f', 'g', 'h',
        'j', 'k', 'l', ';',
        '\'','`',  0 , '\\',
        'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',',
        '.', '/',  0 , '*',
         0 , ' '
};

static void __process_control_keys(uint8_t);
static bool __is_capitalized();
static char __translate_scancode(uint8_t, bool);

void kbd_handle_input(uint8_t scancode)
{
    __process_control_keys(scancode);

    bool caps = __is_capitalized();

    if (scancode == BSPACE_PRESSED) {
        tty_backspace(g_tty);
        return;
    }

    char ascii = __translate_scancode(scancode, caps);
    if (ascii == 0) return;

    tty_putc(g_tty, ascii);
}

static void __process_control_keys(uint8_t scancode)
{
    switch(scancode) {
        case CAPSLOCK_PRESSED:
            _capslock_pressed = !_capslock_pressed;
            break;
        case LSHIFT_PRESSED:
            _lshift_pressed = true;
            break;
        case LSHIFT_RELEASED:
            _lshift_pressed = false;
            break;
        case RSHIFT_PRESSED:
            _rshift_pressed = true;
            break;
        case RSHIFT_RELEASED:
            _rshift_pressed = false;
            break;
        case BSPACE_PRESSED:
            _bspace_pressed = true;
            break;
        case BSPACE_RELEASED:
            _bspace_pressed = false;
            break;
    }
}

static bool __is_capitalized()
{
    bool caps = false;
    if (_capslock_pressed) caps = true;
    if (_rshift_pressed || _lshift_pressed) caps = !caps;
    return caps;
}

static bool __ignore_caps(uint8_t scancode)
{
    //todo: replace with array search
    if (scancode == SPACE_PRESSED) return true;
    return false;
}

static char __translate_scancode(uint8_t scancode, bool caps)
{
    if (scancode > MAX_PRINTABLE_SCANCODE) return 0;
    if (_ascii_table[scancode] == 0) return 0;
    if (__ignore_caps(scancode)) return _ascii_table[scancode];
    return caps ? _ascii_table[scancode] - 32 : _ascii_table[scancode];
}
