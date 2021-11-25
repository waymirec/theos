#include <string.h>

void* memrcpy(void *dest, const void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    while (len--)
        *d++ = *(s + len);
    return dest;
}