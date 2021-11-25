#include <string.h>

void* memzero(void *dest, size_t len)
{
    return memset(dest, 0, len);
}