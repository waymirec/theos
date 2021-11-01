#include "panic.h"
#include "terminal.h"

void panic(char *message)
{
    terminal_clear(0x00000000);
    
}