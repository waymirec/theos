#include <string.h>

size_t strcpy(void *dstptr, const void *srcptr)
{
    unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;

    size_t length = 0;
    while(*src != 0) {
        *dst++ = *src++;
        length++;
    }
    
    return length;
}