#include "bitmap.h"

#define BYTE_INDEX(index) (index / 8)
#define MASK(index) (0b10000000 >> (index % 8))

void bitmap_init(bitmap_t *bitmap, size_t size, void *buffer)
{
    bitmap->size = size;
    bitmap->buffer = (uint8_t *)buffer;
    unsigned char *ptr = buffer;
    size_t len = size;
    while (len-- > 0)
        *ptr++ = 0;
}

bool bitmap_check(bitmap_t *bitmap, uint64_t index)
{
    if (index > bitmap->size * 8) return false;
    return (bitmap->buffer[BYTE_INDEX(index)] & MASK(index)) > 0;
}

bool bitmap_set(bitmap_t *bitmap, uint64_t index)
{
    if (index > bitmap->size * 8) return false;
    bitmap->buffer[BYTE_INDEX(index)] |= MASK(index);
    return true;
}

bool bitmap_clear(bitmap_t *bitmap, uint64_t index)
{
    if (index > bitmap->size * 8) return false;
    bitmap->buffer[BYTE_INDEX(index)] &= ~MASK(index);
    return true;
}
