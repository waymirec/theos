#include <stdio.h>
#include "terminal.h"
int putchar(int ic) {
	char c = (char) ic;
	terminal_put_char(c);
	return ic;
}
