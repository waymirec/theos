#include "string.h"
#include "types.h"

#include <stddef.h>

#define MAX_DBL_PRECISION 15
#define DEC_BASE 10
#define HEX_BASE 16

char *HEX_DIGITS = "0123456789ABCDEF";

int memcmp(const void *string1, const void *string2, size_t length)
{
    const unsigned char *s1 = string1, *s2 = string2;
    for(size_t i = 0; i < length; i++)
    {
        if (s1[i] < s2[i]) return -1;
        if (s1[i] > s2[i]) return 1;
    }
    return 0;
}

void * memcpy(void *dest, void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    while (len--)
        *d++ = *s++;
    return dest;
}

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
    buffer[index] = remainder + '0';
    size_t len = index + 1;
    reverse(buffer, len);
    buffer[len] = 0;
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

    if (value > 0) buffer[index] = HEX_DIGITS[value];

    for(int i=index; i <= 16 - index; i++)
    {
        buffer[++index] = '0';
    }

    reverse(buffer+2, index+1-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index+1] = 0;
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

    if (value > 0) buffer[index] = HEX_DIGITS[value];

    for(int i=index; i <= 8 - index; i++)
    {
        //buffer[++index] = '0';
    }

    reverse(buffer+2, index+1-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index+1] = 0;
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

    if (value > 0) buffer[index] = HEX_DIGITS[value];

    for(int i=index; i <= 4 - index; i++)
    {
        buffer[++index] = '0';
    }

    reverse(buffer+2, index+1-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index+1] = 0;
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

    if (value > 0) buffer[index] = HEX_DIGITS[value];

    for(int i=index; i <= 2 - index; i++)
    {
        buffer[++index] = '0';
    }

    reverse(buffer+2, index+1-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index+1] = 0;
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
    if (len < 2) return;

    char *buff = (char *)buffer;
    size_t end = len-1;
    size_t mid = end / 2;
    if (mid == 0) mid = 1;
    if (mid % 2 == 0) mid++;
    char tmp;
    for(int i=0; i<mid; i++)
    {
        tmp = buff[i];
        buff[i] = buff[end-i];
        buff[end-i] = tmp;
    }
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