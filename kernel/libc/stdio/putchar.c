#include <stdio.h>
#include "globals.h"
#include "tty.h"

int putchar(int ic) {
	char c = (char) ic;
	tty_putc(g_tty, c);
	return ic;
}
