#include "string.h"

#include <stddef.h>

#include "types.h"
#include "memory.h"

#define MAX_DBL_PRECISION 15
#define DEC_BASE 10
#define HEX_BASE 16

char *HEX_DIGITS = "0123456789ABCDEF";


char* uint_to_string(uint64_t value, char *buffer)
{
    uint8_t index = 0;
    while(value >= 10)
    {
        uint8_t remainder = value % DEC_BASE;
        value /= 10;
        buffer[index++] = remainder + '0';
    }

    size_t remainder = value % DEC_BASE;
    buffer[index++] = remainder + '0';
    reverse(buffer, index);
    buffer[index] = 0;
    return buffer;
}

char* int_to_string(int64_t value, char *buffer)
{
    if (value >= 0) return uint_to_string((uint64_t)value, buffer);

    value *= -1;
    uint_to_string((uint64_t)value, buffer+1);
    buffer[0] = '-';
    return buffer;
}

char* uint64_to_hex(uint64_t value, char *buffer)
{
    uint8_t index = 2;
    while (value >= HEX_BASE)
    {
        uint8_t remainder = value % HEX_BASE;
        value /= HEX_BASE;
        buffer[index++] = HEX_DIGITS[remainder];
    }

    if (value > 0) buffer[index++] = HEX_DIGITS[value];

    uint8_t padding = 18 - index;
    for(int i=0; i < padding; i++)
    {
        buffer[index++] = '0';
    }

    reverse(buffer+2, index-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index] = 0;
    return buffer;
}

char* uint32_to_hex(uint32_t value, char *buffer)
{
    uint8_t index = 2;
    while (value >= HEX_BASE)
    {
        uint8_t remainder = value % HEX_BASE;
        value /= HEX_BASE;
        buffer[index++] = HEX_DIGITS[remainder];
    }

    if (value > 0) buffer[index++] = HEX_DIGITS[value];

    uint8_t padding = 10 - index;
    for(int i=0; i < padding; i++)
    {
        buffer[index++] = '0';
    }

    reverse(buffer+2, index-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index] = 0;
    return buffer;
}

char* uint16_to_hex(uint16_t value, char *buffer)
{
    uint8_t index = 2;
    while (value >= HEX_BASE)
    {
        uint8_t remainder = value % HEX_BASE;
        value /= HEX_BASE;
        buffer[index++] = HEX_DIGITS[remainder];
    }

    if (value > 0) buffer[index++] = HEX_DIGITS[value];

    uint8_t padding = 6 - index;
    for(int i=0; i < padding; i++)
    {
        buffer[index++] = '0';
    }

    reverse(buffer+2, index-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index] = 0;
    return buffer;
}

char* uint8_to_hex(uint8_t value, char *buffer)
{
    uint8_t index = 2;
    while (value >= HEX_BASE)
    {
        uint8_t remainder = value % HEX_BASE;
        value /= HEX_BASE;
        buffer[index++] = HEX_DIGITS[remainder];
    }

    if (value > 0) buffer[index++] = HEX_DIGITS[value];

    uint8_t padding = 4 - index;
    for(int i=0; i < padding; i++)
    {
        buffer[index++] = '0';
    }

    reverse(buffer+2, index-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index] = 0;
    return buffer;
}
char* int_to_hex(int64_t value, char *buffer)
{
    return uint64_to_hex((uint64_t)value, buffer);
}

char* double_to_string(double value, uint8_t precision, char *buffer)
{
    if (precision > MAX_DBL_PRECISION) precision = MAX_DBL_PRECISION;
    uint64_t multiplier = 1 ;
    for(int i=0; i < precision; i++) multiplier *= 10;

    size_t len = 0;
    char whole_buffer[128];
    int_to_string((int64_t)value, whole_buffer);
    size_t whole_len = strlen(whole_buffer);
    strcpy(buffer, whole_buffer);
    buffer[whole_len] = '.';
    len += whole_len + 1;

    if (value < 0) value *= -1;
    double decimalValue = (value - (uint64_t)value) * multiplier;
    char decimal_buffer[128];
    uint_to_string((uint64_t)decimalValue, decimal_buffer);
    size_t decimal_len = strlen(decimal_buffer);
    memcpy(buffer+len, decimal_buffer, decimal_len);
    len += decimal_len;

    buffer[len] = 0;
    return buffer;
}

void reverse(void *buffer, size_t len)
{
    uint8_t tmp[len];
    memrcpy((void *)tmp, buffer, len);
    memcpy(buffer, tmp, len);
}

char * strcpy(char *dest, char *src)
{
    return memcpy(dest, src, strlen(src)+1);
}

size_t strrev(char *string)
{
    size_t len = strlen(string);
    reverse((void *)string, len);
    string[len] = 0;
    return len;
}

size_t strlen(char *string)
{
    size_t len = 0;
    while(string[len] != 0)
    {
        len++;
    }
    return len;
}