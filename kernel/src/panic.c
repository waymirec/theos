#include "panic.h"

#include <stdio.h>

#include "globals.h"
#include "tty.h"

void panic(char *message)
{
    tty_clear(g_tty);
    printf("Kernel Panic!!\n");
    printf(message);
}