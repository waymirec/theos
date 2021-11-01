#include "panic.h"
#include "terminal.h"

void panic(char *message)
{
    terminal_clear(0x00000000);
    terminal_println("Kernel Panic!!");
    terminal_newline();
    terminal_println(message);
}